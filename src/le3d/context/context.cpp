#include <iostream>
#include <glad/glad.h>
#include "le3d/core/log.hpp"
#include "le3d/context/context.hpp"
#include "le3d/input/inputImpl.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/gfx/utils.hpp"
#include "contextImpl.hpp"

namespace le
{
namespace context
{
std::thread::id g_contextThreadID;
std::mutex g_glMutex;
} // namespace context

namespace
{
glm::vec2 g_windowSize;
f32 g_nativeAR = 1.0f;
GLFWwindow* g_pRenderWindow = nullptr;

void glframeBufferResizeCallback(GLFWwindow* pWindow, s32 width, s32 height)
{
	Lock lock(context::g_glMutex);
	if (pWindow == g_pRenderWindow)
	{
		g_windowSize = {width, height};
		g_nativeAR = (f32)width / height;
		glViewport(0, 0, width, height);
	}
}

void onError(s32 code, const char* szDesc)
{
	LOG_E("GLFW Error [%d]: %s", code, szDesc);
}
} // namespace

bool context::glCreate(u16 width, u16 height, std::string_view title)
{
	g_windowSize = glm::vec2(width, height);
	glfwSetErrorCallback(&onError);
	if (!glfwInit())
	{
		LOG_E("Failed to initialise GLFW!");
		return false;
	}
	g_pRenderWindow = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	if (!g_pRenderWindow)
	{
		LOG_E("Failed to create window!");
		return false;
	}
	{
		Lock lock(g_glMutex);
		glfwMakeContextCurrent(g_pRenderWindow);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			LOG_E("Failed to initialise GLAD!");
			return false;
		}
		inputImpl::init(*g_pRenderWindow);
		g_contextThreadID = std::this_thread::get_id();
		glEnable(GL_DEPTH_TEST);
	}
	glframeBufferResizeCallback(g_pRenderWindow, width, height);
	glfwSetFramebufferSizeCallback(g_pRenderWindow, &glframeBufferResizeCallback);
	LOG_D("Context created");
	return true;
}

void context::glDestroy()
{
	le::resources::unloadShaders();
	le::resources::unloadTextures(true);
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

void context::glClearFlags(Colour colour /* = Colour::Black */, u32 flags /* = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT */)
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
