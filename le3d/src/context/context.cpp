#include <iostream>
#include <memory>
#include <glad/glad.h>
#include "le3d/engineVersion.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/fileLogger.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/context/context.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/gfx/utils.hpp"
#include "contextImpl.hpp"
#include "input/inputImpl.hpp"

namespace le
{
namespace contextImpl
{
std::thread::id g_contextThreadID;
std::mutex g_glMutex;
} // namespace contextImpl

using namespace contextImpl;

namespace
{
std::unique_ptr<FileLogger> g_uFileLogger;
glm::vec2 g_windowSize;
f32 g_nativeAR = 1.0f;
GLFWwindow* g_pRenderWindow = nullptr;

void glframeBufferResizeCallback(GLFWwindow* pWindow, s32 width, s32 height)
{
	Lock lock(g_glMutex);
	if (pWindow == g_pRenderWindow)
	{
		g_windowSize = {width, height};
		g_nativeAR = height > 0 ? (f32)width / height : 0.0f;
		glViewport(0, 0, width, height);
		inputImpl::g_callbacks.onResize(width, height);
	}
}

void onError(s32 code, const char* szDesc)
{
	LOG_E("GLFW Error [%d]: %s", code, szDesc);
}
} // namespace

bool context::create(const Settings& settings)
{
	if (settings.logOpts.bLogToFile)
	{
		auto path = env::dirPath(settings.logOpts.dir) / settings.logOpts.filename;
		g_uFileLogger = std::make_unique<FileLogger>(std::move(path));
	}
	LOG_I("LittleEngine3D v%s", versions::buildVersion().data());
	glfwSetErrorCallback(&onError);
	if (!glfwInit())
	{
		LOG_E("Failed to initialise GLFW!");
		return false;
	}

	s32 screenCount;
	GLFWmonitor** ppScreens = glfwGetMonitors(&screenCount);
	if (screenCount < 1)
	{
		LOG_E("Failed to detect output device");
		return false;
	}
	const GLFWvidmode* mode = glfwGetVideoMode(ppScreens[0]);
	if (!mode)
	{
		LOG_E("Failed to get default screen's video mode");
		return false;
	}
	GLFWmonitor* pTarget = nullptr;
	u16 height = settings.height;
	u16 width = settings.width;
	s32 screenIdx = settings.screenID < screenCount ? (s32)settings.screenID : -1;
	bool bVSYNC = settings.bVSYNC;
	switch (settings.type)
	{
	case Type::BorderedWindow:
	{
		if (mode->width < width || mode->height < height)
		{
			LOG_E("Context size [%ux%u] too large for default screen! [%ux%u]", width, height, mode->width, mode->height);
			return false;
		}
		break;
	}
	case Type::BorderlessFullscreen:
	{
		height = (u16)mode->height;
		width = (u16)mode->width;
		pTarget = ppScreens[(size_t)screenIdx];
		break;
	}
	case Type::Dedicated:
	{
		pTarget = ppScreens[(size_t)screenIdx];
		break;
	}
	}
	g_windowSize = glm::vec2(width, height);
	s32 cX = (mode->width - width) / 2;
	s32 cY = (mode->height - height) / 2;
	ASSERT(cX >= 0 && cY >= 0, "Invalid centre-screen!");

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	g_pRenderWindow = glfwCreateWindow(width, height, settings.title.data(), pTarget, nullptr);
	if (!g_pRenderWindow)
	{
		LOG_E("Failed to create window!");
		return false;
	}
	glfwSetWindowPos(g_pRenderWindow, cX, cY);
	{
		Lock lock(g_glMutex);
		glfwMakeContextCurrent(g_pRenderWindow);
#if defined(FORCE_NO_VSYNC)
		bVSYNC = false;
#endif
		LOGIF_I(!bVSYNC, "[Context] Vsync disabled unless overridden by driver");
		glfwSwapInterval(bVSYNC ? 1 : 0);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			LOG_E("Failed to initialise GLAD!");
			return false;
		}
		inputImpl::init(*g_pRenderWindow);
		g_contextThreadID = std::this_thread::get_id();
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glframeBufferResizeCallback(g_pRenderWindow, width, height);
	glfwSetFramebufferSizeCallback(g_pRenderWindow, &glframeBufferResizeCallback);
	LOG_D("Context created");
	return true;
}

void context::destroy()
{
	le::resources::unloadAll();
	Lock lock(g_glMutex);
	if (g_pRenderWindow)
	{
		glfwSetWindowShouldClose(g_pRenderWindow, true);
		while (!glfwWindowShouldClose(g_pRenderWindow))
		{
			glfwPollEvents();
		}
	}
	glfwTerminate();
	g_pRenderWindow = nullptr;
	g_windowSize = glm::vec2(0.0f);
	g_contextThreadID = std::thread::id();
	LOG_D("Context destroyed");
	g_uFileLogger = nullptr;
}

bool context::exists()
{
	return g_pRenderWindow != nullptr;
}

bool context::isClosing()
{
	return g_pRenderWindow ? glfwWindowShouldClose(g_pRenderWindow) : false;
}

void context::clearFlags(u32 flags, Colour colour /* = Colour::Black */)
{
	Lock lock(g_glMutex);
	glChk(glClearColor(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
	glClear(flags);
}

void context::pollEvents()
{
	if (g_pRenderWindow)
	{
		glfwPollEvents();
	}
}

void context::swapBuffers()
{
	if (g_pRenderWindow)
	{
		glfwSwapBuffers(g_pRenderWindow);
	}
}

f32 context::nativeAR()
{
	return g_nativeAR;
}

glm::vec2 context::size()
{
	return g_windowSize;
}

glm::vec2 context::project(glm::vec2 nPos, glm::vec2 space)
{
	return {nPos.x * space.x, nPos.y * space.y};
}

glm::vec2 context::projectScreen(glm::vec2 nPos)
{
	return project(nPos, g_windowSize);
}

glm::vec2 context::worldToScreen(glm::vec2 world)
{
	return g_windowSize == glm::vec2(0.0f) ? g_windowSize : glm::vec2(world.x / g_windowSize.x, world.y / g_windowSize.y);
}
} // namespace le
