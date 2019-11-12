#include <iostream>
#include <glad/glad.h>
#include "le3d/log/log.hpp"
#include "le3d/context/context.hpp"
#include "le3d/input/inputImpl.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
namespace
{
Vector2 g_windowSize;
GLFWwindow* g_pRenderWindow = nullptr;

void frameBufferResizeCallback(GLFWwindow* pWindow, s32 width, s32 height)
{
	if (pWindow == g_pRenderWindow)
	{
		glViewport(0, 0, width, height);
	}
}

void onError(s32 code, const char* szDesc)
{
	std::cerr << "GLFW Error [" << code << "]: " << std::string(szDesc) << std::endl;
}
} // namespace

bool context::create(u16 width, u16 height, std::string_view title)
{
	g_windowSize = Vector2(width, height);
	glfwSetErrorCallback(&onError);
	if (!glfwInit())
	{
		logE("Failed to initialise GLFW!");
		return false;
	}
	g_pRenderWindow = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	if (!g_pRenderWindow)
	{
		logE("Failed to create window!");
		return false;
	}
	glfwSetFramebufferSizeCallback(g_pRenderWindow, &frameBufferResizeCallback);
	glfwMakeContextCurrent(g_pRenderWindow);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		logE("Failed to initialise GLAD!");
		return false;
	}
	inputImpl::init(*g_pRenderWindow);
	glEnable(GL_DEPTH_TEST);
	logD("Context created");
	return true;
}

void context::destroy()
{
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
	g_windowSize = Vector2::Zero;
	logD("Context destroyed");
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
	glClearColor(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32());
	glChk();
	glClear(flags);
	glChk();
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
	glBindTexture(GL_TEXTURE_2D, 0);
	if (g_pRenderWindow)
	{
		glfwSwapBuffers(g_pRenderWindow);
	}
}

Vector2 context::size()
{
	return g_windowSize;
}

Vector2 context::project(Vector2 nPos, Vector2 space)
{
	return {nPos.x * space.x, nPos.y * space.y};
}

Vector2 context::projectScreen(Vector2 nPos)
{
	return project(nPos, g_windowSize);
}

Vector2 context::worldToScreen(Vector2 world)
{
	return g_windowSize == Vector2::Zero ? g_windowSize : Vector2(world.x / g_windowSize.x, world.y / g_windowSize.y);
}
} // namespace le
