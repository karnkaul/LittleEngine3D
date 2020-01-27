#include "le3d/core/assert.hpp"
#include "le3d/core/jobs.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/env/env.hpp"
#include "le3d/env/threads.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/le3dgl.hpp"
#include "le3d/game/resources.hpp"
#include "core/ioImpl.hpp"
#include "inputImpl.hpp"
#include "contextImpl.hpp"
#if defined(LE3D_USE_GLFW)
#include <GLFW/glfw3.h>
#endif

namespace le
{
#if !defined(LE3D_USE_GLFW)

bool contextImpl::init(context::Settings const&)
{
	ASSERT(false, "Unsupported platform!");
	return false;
}
void contextImpl::checkContextThread() {}
bool contextImpl::isAlive()
{
	return false;
}
void contextImpl::close() {}
bool contextImpl::isClosing()
{
	return false;
}
bool contextImpl::exists()
{
	return false;
}
void contextImpl::clearFlags(context::ClearFlags, Colour) {}
void contextImpl::pollEvents() {}
void contextImpl::setSwapInterval(u8) {}
void contextImpl::swapBuffers() {}
void contextImpl::setPolygonMode(context::PolygonFace, context::PolygonMode) {}
void contextImpl::toggle(context::GFXFlag, bool) {}
void contextImpl::destroy() {}

#else

namespace
{
GLFWwindow* g_pWindow = nullptr;

void glframeBufferResizeCallback(GLFWwindow* pWindow, s32 width, s32 height)
{
	if (pWindow == g_pWindow)
	{
		contextImpl::g_context.size = {width, height};
		contextImpl::g_context.nativeAR = height > 0 ? (f32)width / height : 0.0f;
		glViewport(0, 0, width, height);
		inputImpl::g_callbacks.onResize(width, height);
	}
}

void windowCloseCallback(GLFWwindow* pWindow)
{
	if (pWindow == g_pWindow)
	{
		LOG_I("[Context] Window closed, terminating session");
		inputImpl::g_callbacks.onClosed();
	}
}

void onError(s32 code, char const* szDesc)
{
	LOG_E("GLFW Error [%d]: %s", code, szDesc);
}
} // namespace

bool contextImpl::init(context::Settings const& settings)
{
	glfwSetErrorCallback(&onError);
	if (!glfwInit())
	{
		LOG_E("FATAL: Failed to initialise GLFW!");
		return {};
	}

	s32 screenCount;
	GLFWmonitor** ppScreens = glfwGetMonitors(&screenCount);
	if (screenCount < 1)
	{
		LOG_E("FATAL: Failed to detect output device");
		return {};
	}
	GLFWvidmode const* mode = glfwGetVideoMode(ppScreens[0]);
	if (!mode)
	{
		LOG_E("FATAL: Failed to get default screen's video mode");
		return {};
	}
	GLFWmonitor* pTarget = nullptr;
	u16 height = settings.window.height;
	u16 width = settings.window.width;
	s32 screenIdx = settings.window.screenID < screenCount ? (s32)settings.window.screenID : -1;
	bool bVSYNC = settings.ctxt.bVSYNC;
	bool bDecorated = true;
	switch (settings.window.type)
	{
	default:
	case context::WindowType::DecoratedWindow:
	{
		if (mode->width < width || mode->height < height)
		{
			LOG_E("FATAL: Context size [%ux%u] too large for default screen! [%ux%u]", width, height, mode->width, mode->height);
			return {};
		}
		break;
	}
	case context::WindowType::BorderlessWindow:
	{
		if (mode->width < width || mode->height < height)
		{
			LOG_E("FATAL: Context size [%ux%u] too large for default screen! [%ux%u]", width, height, mode->width, mode->height);
			return {};
		}
		bDecorated = false;
		break;
	}
	case context::WindowType::BorderlessFullscreen:
	{
		height = (u16)mode->height;
		width = (u16)mode->width;
		pTarget = ppScreens[(size_t)screenIdx];
		break;
	}
	case context::WindowType::DedicatedFullscreen:
	{
		pTarget = ppScreens[(size_t)screenIdx];
		break;
	}
	}
	g_context.size = glm::vec2(width, height);
	s32 cX = (mode->width - width) / 2;
	s32 cY = (mode->height - height) / 2;
	ASSERT(cX >= 0 && cY >= 0, "Invalid centre-screen!");
	glfwWindowHint(GLFW_DECORATED, bDecorated ? 1 : 0);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	glfwWindowHint(GLFW_VISIBLE, false);
	auto pWindow = glfwCreateWindow(width, height, settings.window.title.data(), pTarget, nullptr);
	if (!pWindow)
	{
		LOG_E("FATAL: Failed to create window!");
		return {};
	}
	glfwMakeContextCurrent(pWindow);
#if defined(LE3D_USE_GLAD)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LOG_E("FATAL: Failed to load OpenGL function pointers!");
		glfwTerminate();
		return {};
	}
#else
	LOG_E("FATAL: No OpenGL loader exists!");
	glfwTerminate();
	return {};
#endif
	auto szVersion = (char const*)glGetString(GL_VERSION);
	auto versionStr = utils::strings::bisect(szVersion, ' ');
	Version glVersion(versionStr.first);
	if (glVersion < settings.ctxt.minVersion)
	{
		auto const& v = settings.ctxt.minVersion;
		LOG_E("FATAL: Incompatible OpenGL context: [%s]; required: [%s]", glVersion.toString().data(), v.toString().data());
		glfwTerminate();
		return {};
	}
	if (!inputImpl::init(*pWindow))
	{
		LOG_E("FATAL: Failed to initialise input!");
		return {};
	}
	// Success!
#if defined(FORCE_NO_VSYNC)
	bVSYNC = false;
#endif
	g_pWindow = pWindow;
	setSwapInterval(bVSYNC ? 1 : 0);
	glfwSetWindowPos(g_pWindow, cX, cY);
	glfwShowWindow(g_pWindow);
	g_contextThreadID = std::this_thread::get_id();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glframeBufferResizeCallback(g_pWindow, width, height);
	glfwSetFramebufferSizeCallback(g_pWindow, &glframeBufferResizeCallback);
	if (settings.env.jobWorkerCount > 0)
	{
		jobs::init(settings.env.jobWorkerCount);
	}
	glfwSetWindowCloseCallback(g_pWindow, &windowCloseCallback);
	LOG_I("== [%s] OpenGL context created using %s [%s]", glVersion.toString().data(), glGetString(GL_RENDERER), glGetString(GL_VENDOR));
	return true;
}

void contextImpl::checkContextThread()
{
	auto const& tid = std::this_thread::get_id();
	ASSERT(tid == g_contextThreadID, "Non-context thread attempting access!");
	if (tid != g_contextThreadID)
	{
		LOG_E("[Context] Invalid thread id attempting to access context!");
	}
}

bool contextImpl::isAlive()
{
	return g_pWindow && !isClosing();
}

void contextImpl::close()
{
	if (isAlive())
	{
		glfwSetWindowShouldClose(g_pWindow, true);
	}
}

bool contextImpl::isClosing()
{
	return g_pWindow ? glfwWindowShouldClose(g_pWindow) : false;
}

bool contextImpl::exists()
{
	return g_pWindow != nullptr;
}

void contextImpl::clearFlags(context::ClearFlags flags, Colour colour)
{
	if (g_pWindow)
	{
		cxChk();
		GLbitfield glFlags = 0;
		if (flags.isSet(context::ClearFlag::ColorBuffer))
		{
			glChk(glClearColor(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
			glFlags |= GL_COLOR_BUFFER_BIT;
		}
		if (flags.isSet(context::ClearFlag::DepthBuffer))
		{
			glFlags |= GL_DEPTH_BUFFER_BIT;
		}
		if (flags.isSet(context::ClearFlag::StencilBuffer))
		{
			glFlags |= GL_STENCIL_BUFFER_BIT;
		}
		glClear(glFlags);
	}
}

void contextImpl::pollEvents()
{
	if (g_pWindow)
	{
		glfwPollEvents();
	}
}

void contextImpl::setSwapInterval(u8 interval)
{
	g_context.swapInterval = interval;
#if defined(FORCE_NO_VSYNC)
	g_context.swapInterval = 0;
#endif
	glfwSwapInterval(s32(g_context.swapInterval));
}

void contextImpl::swapBuffers()
{
	if (g_pWindow)
	{
		glfwSwapBuffers(g_pWindow);
		++g_context.swapCount;
	}
}

void contextImpl::setPolygonMode(context::PolygonFace face, context::PolygonMode mode)
{
	GLenum glFace, glMode;
	switch (face)
	{
	default:
	case context::PolygonFace::FrontAndBack:
		glFace = GL_FRONT_AND_BACK;
		break;
	case context::PolygonFace::Front:
		glFace = GL_FRONT;
		break;
	case context::PolygonFace::Back:
		glFace = GL_BACK;
		break;
	}
	switch (mode)
	{
	default:
	case context::PolygonMode::Fill:
		glMode = GL_FILL;
		break;
	case context::PolygonMode::Line:
		glMode = GL_LINE;
		break;
	}
	glPolygonMode(glFace, glMode);
}

void contextImpl::toggle(context::GFXFlag flag, bool bEnable)
{
	switch (flag)
	{
	default:
		return;
	case context::GFXFlag::DepthTest:
		if (bEnable)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		break;
	}
}

void contextImpl::destroy()
{
	if (g_pWindow)
	{
		LOG_D("[Context] Destroying context, terminating session...");
		cxChk();
		inputImpl::clear();
		le::resources::unloadAll();
		glfwSetWindowShouldClose(g_pWindow, true);
		while (!glfwWindowShouldClose(g_pWindow))
		{
			glfwPollEvents();
		}
		glfwTerminate();
		g_pWindow = nullptr;
		jobs::cleanup();
		bool bJoinThreads = g_context.bJoinThreadsOnDestroy;
		g_contextThreadID = std::thread::id();
		LOG_I("-- Context destroyed");
		ioImpl::deinitPhysfs();
		g_context = LEContext();
		if (bJoinThreads)
		{
			threads::joinAll();
		}
	}
}

#endif
} // namespace le
