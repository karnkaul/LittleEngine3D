#include <iostream>
#include <glad/glad.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
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

namespace context
{
bool g_bVSYNC = true;
} // namespace context

using namespace contextImpl;

namespace
{
glm::vec2 g_windowSize;
f32 g_nativeAR = 1.0f;
GLFWwindow* g_pRenderWindow = nullptr;

void glframeBufferResizeCallback(GLFWwindow* pWindow, s32 width, s32 height)
{
	Lock lock(g_glMutex);
	if (pWindow == g_pRenderWindow)
	{
		g_windowSize = {width, height};
		g_nativeAR = (f32)width / height;
		glViewport(0, 0, width, height);
		inputImpl::g_callbacks.onResize(width, height);
	}
}

void onError(s32 code, const char* szDesc)
{
	LOG_E("GLFW Error [%d]: %s", code, szDesc);
}
} // namespace

bool context::create(u16 width, u16 height, std::string_view title)
{
	glfwSetErrorCallback(&onError);
	if (!glfwInit())
	{
		LOG_E("Failed to initialise GLFW!");
		return false;
	}

	s32 screenCount;
	GLFWmonitor** screens = glfwGetMonitors(&screenCount);
	if (screenCount < 1)
	{
		LOG_E("Failed to detect output device");
		return false;
	}
	const GLFWvidmode* mode = glfwGetVideoMode(screens[0]);
	if (!mode)
	{
		LOG_E("Failed to get default screen's video mode");
		return false;
	}
	if (mode->width < width || mode->height < height)
	{
		LOG_E("Context size [%ux%u] too large for default screen! [%ux%u]", width, height, mode->width, mode->height);
		return false;
	}
	g_windowSize = glm::vec2(width, height);
	s32 cX = (mode->width - width) / 2;
	s32 cY = (mode->height - height) / 2;
	ASSERT(cX > 0 && cY > 0, "Invalid centre-screen!");

	g_pRenderWindow = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
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
		g_bVSYNC = false;
#endif
		LOGIF_I(!g_bVSYNC, "[Context] Vsync disabled unless overridden by driver");
		glfwSwapInterval(g_bVSYNC ? 1 : 0);
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
}

bool context::exists()
{
	return g_pRenderWindow != nullptr;
}

bool context::isClosing()
{
	return g_pRenderWindow ? glfwWindowShouldClose(g_pRenderWindow) : false;
}

void context::clearFlags(Colour colour /* = Colour::Black */, u32 flags /* = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT */)
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