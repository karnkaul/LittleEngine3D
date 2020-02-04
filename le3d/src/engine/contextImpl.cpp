#include "le3d/core/assert.hpp"
#include "le3d/core/jobs.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/env/env.hpp"
#include "le3d/env/threads.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfxThread.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/game/resources.hpp"
#include "core/ioImpl.hpp"
#include "inputImpl.hpp"
#include "contextImpl.hpp"
#if defined(LE3D_USE_GLFW)
#include <GLFW/glfw3.h>
#endif

namespace le
{
namespace gfx
{
extern u64 g_renderSwapCount;
}

#if !defined(LE3D_USE_GLFW)

bool contextImpl::init(context::Settings const&)
{
	ASSERT(false, "Unsupported platform!");
	return false;
}
void contextImpl::checkContextThread()
{
	return;
}
bool contextImpl::isAlive()
{
	return false;
}
void contextImpl::close()
{
	return;
}
bool contextImpl::isClosing()
{
	return false;
}
bool contextImpl::exists()
{
	return false;
}
void contextImpl::clearFlags(context::ClearFlags, Colour)
{
	return;
}
void contextImpl::pollEvents()
{
	return;
}
void contextImpl::setSwapInterval(u8)
{
	return;
}
void contextImpl::swapBuffers()
{
	return;
}
void contextImpl::setPolygonMode(context::PolygonFace, context::PolygonMode)
{
	return;
}
void contextImpl::toggle(context::GFXFlag, bool)
{
	return;
}
void contextImpl::destroy()
{
	return;
}

#else

namespace
{
GLFWwindow* g_pWindow = nullptr;

void glframeBufferResizeCallback(GLFWwindow* pWindow, s32 width, s32 height)
{
	if (pWindow == g_pWindow)
	{
		contextImpl::g_context.windowSize = {width, height};
		contextImpl::g_context.windowAR = height > 0 ? (f32)width / height : 0.0f;
		// TODO: Store somewhere else, don't immediately set? Might be cropped...
		gfx::setViewport(0, 0, width, height);
		inputImpl::g_callbacks.onResize(width, height);
	}
	return;
}

void windowCloseCallback(GLFWwindow* pWindow)
{
	if (pWindow == g_pWindow)
	{
		LOG_I("[Context] Window closed, terminating session");
		inputImpl::g_callbacks.onClosed();
	}
	return;
}

void onError(s32 code, char const* szDesc)
{
	LOG_E("GLFW Error [%d]: %s", code, szDesc);
	return;
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
	g_context.windowSize = glm::vec2(width, height);
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
	if (!gfx::loadFunctionPointers((gfx::GLLoadProc)glfwGetProcAddress))
	{
		LOG_E("FATAL: Failed to load OpenGL function pointers!");
		glfwTerminate();
		return {};
	}
	auto versionStr = utils::strings::bisect(gfx::getString(StringProp::Version), ' ');
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
	gfx::setFlag(GLFlag::DepthTest, true);
	gfx::setFlag(GLFlag::Blend, true);
	gfx::setBlendFunc(BlendFunc::Src_OneMinusSrc);
	glframeBufferResizeCallback(g_pWindow, width, height);
	glfwSetFramebufferSizeCallback(g_pWindow, &glframeBufferResizeCallback);
	if (settings.env.jobWorkerCount > 0)
	{
		jobs::init(settings.env.jobWorkerCount);
	}
	glfwSetWindowCloseCallback(g_pWindow, &windowCloseCallback);
	auto renderer = gfx::getString(StringProp::Renderer);
	auto vendor = gfx::getString(StringProp::Vendor);
	LOG_I("== [%s] OpenGL context created using %s [%s]", glVersion.toString().data(), renderer.data(), vendor.data());
	if (settings.ctxt.bThreaded)
	{
		gfx::startThread();
	}
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
	return;
}

void contextImpl::setCurrentContext()
{
	if (exists())
	{
		glfwMakeContextCurrent(g_pWindow);
	}
	return;
}

void contextImpl::releaseCurrentContext()
{
	if (exists())
	{
		glfwMakeContextCurrent(nullptr);
	}
	return;
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
	return;
}

bool contextImpl::isClosing()
{
	return g_pWindow ? glfwWindowShouldClose(g_pWindow) : false;
}

bool contextImpl::exists()
{
	return g_pWindow != nullptr;
}

void contextImpl::pollEvents()
{
	if (g_pWindow)
	{
		glfwPollEvents();
	}
	return;
}

void contextImpl::setSwapInterval(u8 interval)
{
	g_context.swapInterval = interval;
#if defined(FORCE_NO_VSYNC)
	g_context.swapInterval = 0;
#endif
	gfx::enqueue([]() { glfwSwapInterval(s32(g_context.swapInterval)); });
	return;
}

void contextImpl::swapBuffers()
{
	if (g_pWindow)
	{
		auto f = gfx::enqueue([]() {
			glfwSwapBuffers(g_pWindow);
			++gfx::g_renderSwapCount;
		});
		gfx::wait(f);
		++g_context.swapCount;
	}
	return;
}

void contextImpl::destroy()
{
	if (g_pWindow)
	{
		LOG_D("[Context] Destroying context, terminating session...");
		inputImpl::clear();
		le::resources::unloadAll();
		if (gfx::isThreadRunning())
		{
			gfx::stopThread();
		}
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
	return;
}

#endif
} // namespace le
