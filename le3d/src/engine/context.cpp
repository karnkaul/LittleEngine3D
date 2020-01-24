#include <iostream>
#include <memory>
#include <glad/glad.h>
#include "le3d/env/engineVersion.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/fileLogger.hpp"
#include "le3d/core/jobs.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "core/ioImpl.hpp"
#include "contextImpl.hpp"
#include "inputImpl.hpp"

namespace le
{
namespace
{
struct LEContext
{
	std::unique_ptr<FileLogger> uFileLogger;
	glm::vec2 size;
	GLFWwindow* pWindow = nullptr;
	f32 nativeAR = 1.0f;
	u64 swapCount = 0;
	bool bJoinThreadsOnDestroy = true;
};

LEContext g_context;

void glframeBufferResizeCallback(GLFWwindow* pWindow, s32 width, s32 height)
{
	if (pWindow == g_context.pWindow)
	{
		g_context.size = {width, height};
		g_context.nativeAR = height > 0 ? (f32)width / height : 0.0f;
		glViewport(0, 0, width, height);
		inputImpl::g_callbacks.onResize(width, height);
	}
}

void windowCloseCallback(GLFWwindow* pWindow)
{
	if (pWindow == g_context.pWindow)
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

void contextImpl::checkContextThread()
{
	auto const& tid = std::this_thread::get_id();
	ASSERT(tid == g_contextThreadID, "Non-context thread attempting access!");
	if (tid != g_contextThreadID)
	{
		LOG_E("[Context] Invalid thread id attempting to access context!");
	}
}

context::HContext::~HContext()
{
	contextImpl::destroy();
}

std::unique_ptr<context::HContext> context::create(Settings const& settings)
{
	std::unique_ptr<FileLogger> uFileLogger;
	env::init(settings.env.args);
	if (settings.log.bLogToFile)
	{
		auto path = env::dirPath(settings.log.dir) / settings.log.filename;
		uFileLogger = std::make_unique<FileLogger>(std::move(path));
	}
	LOG_I("LittleEngine3D v%s", env::buildVersion().data());
	glfwSetErrorCallback(&onError);
	if (!glfwInit())
	{
		LOG_E("Failed to initialise GLFW!");
		return {};
	}

	s32 screenCount;
	GLFWmonitor** ppScreens = glfwGetMonitors(&screenCount);
	if (screenCount < 1)
	{
		LOG_E("Failed to detect output device");
		return {};
	}
	GLFWvidmode const* mode = glfwGetVideoMode(ppScreens[0]);
	if (!mode)
	{
		LOG_E("Failed to get default screen's video mode");
		return {};
	}
	GLFWmonitor* pTarget = nullptr;
	u16 height = settings.window.height;
	u16 width = settings.window.width;
	s32 screenIdx = settings.window.screenID < screenCount ? (s32)settings.window.screenID : -1;
	bool bVSYNC = settings.window.bVSYNC;
	bool bDecorated = true;
	switch (settings.window.type)
	{
	default:
	case WindowType::DecoratedWindow:
	{
		if (mode->width < width || mode->height < height)
		{
			LOG_E("Context size [%ux%u] too large for default screen! [%ux%u]", width, height, mode->width, mode->height);
			return {};
		}
		break;
	}
	case WindowType::BorderlessWindow:
	{
		if (mode->width < width || mode->height < height)
		{
			LOG_E("Context size [%ux%u] too large for default screen! [%ux%u]", width, height, mode->width, mode->height);
			return {};
		}
		bDecorated = false;
		break;
	}
	case WindowType::BorderlessFullscreen:
	{
		height = (u16)mode->height;
		width = (u16)mode->width;
		pTarget = ppScreens[(size_t)screenIdx];
		break;
	}
	case WindowType::DedicatedFullscreen:
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
	g_context.pWindow = glfwCreateWindow(width, height, settings.window.title.data(), pTarget, nullptr);
	if (!g_context.pWindow)
	{
		LOG_E("Failed to create window!");
		return {};
	}
	glfwSetWindowPos(g_context.pWindow, cX, cY);
	glfwShowWindow(g_context.pWindow);
	glfwMakeContextCurrent(g_context.pWindow);
#if defined(FORCE_NO_VSYNC)
	bVSYNC = false;
#endif
	glfwSwapInterval(bVSYNC ? 1 : 0);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LOG_E("Failed to initialise GLAD!");
		return {};
	}
	inputImpl::init(*g_context.pWindow);
	contextImpl::g_contextThreadID = std::this_thread::get_id();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glframeBufferResizeCallback(g_context.pWindow, width, height);
	glfwSetFramebufferSizeCallback(g_context.pWindow, &glframeBufferResizeCallback);

	if (settings.env.jobWorkerCount > 0)
	{
		jobs::init(settings.env.jobWorkerCount);
	}
	glfwSetWindowCloseCallback(g_context.pWindow, &windowCloseCallback);
	LOGIF_I(!bVSYNC, "[Context] Vsync disabled unless overridden by driver");
	LOG_D("Context created");
	g_context.bJoinThreadsOnDestroy = settings.bJoinThreadsOnDestroy;
	g_context.uFileLogger = std::move(uFileLogger);
	return std::make_unique<HContext>();
}

bool context::isAlive()
{
	return g_context.pWindow && !isClosing();
}

void context::close()
{
	if (isAlive())
	{
		glfwSetWindowShouldClose(g_context.pWindow, true);
	}
}

bool context::isClosing()
{
	return g_context.pWindow ? glfwWindowShouldClose(g_context.pWindow) : false;
}

void context::clearFlags(u32 flags, Colour colour /* = Colour::Black */)
{
	if (g_context.pWindow)
	{
		cxChk();
		glChk(glClearColor(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
		glClear(flags);
	}
}

void context::pollEvents()
{
	if (g_context.pWindow)
	{
		glfwPollEvents();
	}
}

void context::swapBuffers()
{
	if (g_context.pWindow)
	{
		glfwSwapBuffers(g_context.pWindow);
		++g_context.swapCount;
	}
}

u64 context::swapCount()
{
	return g_context.swapCount;
}

f32 context::nativeAR()
{
	return g_context.nativeAR;
}

glm::vec2 context::size()
{
	return g_context.size;
}

glm::vec2 context::project(glm::vec2 nPos, glm::vec2 space)
{
	return {nPos.x * space.x, nPos.y * space.y};
}

glm::vec2 context::projectScreen(glm::vec2 nPos)
{
	return project(nPos, g_context.size);
}

glm::vec2 context::worldToScreen(glm::vec2 world)
{
	return g_context.size == glm::vec2(0.0f) ? g_context.size : glm::vec2(world.x / g_context.size.x, world.y / g_context.size.y);
}

bool contextImpl::exists()
{
	return g_context.pWindow != nullptr;
}

void contextImpl::destroy()
{
	if (g_context.pWindow)
	{
		LOG_D("[Context] Destroying context, terminating session...");
		cxChk();
		inputImpl::clear();
		le::resources::unloadAll();
		if (g_context.pWindow)
		{
			glfwSetWindowShouldClose(g_context.pWindow, true);
			while (!glfwWindowShouldClose(g_context.pWindow))
			{
				glfwPollEvents();
			}
		}
		glfwTerminate();
		jobs::cleanup();
		bool bJoinThreads = g_context.bJoinThreadsOnDestroy;
		g_contextThreadID = std::thread::id();
		LOG_D("Context destroyed");
		ioImpl::deinitPhysfs();
		g_context = LEContext();
		if (bJoinThreads)
		{
			threads::joinAll();
		}
	}
}
} // namespace le
