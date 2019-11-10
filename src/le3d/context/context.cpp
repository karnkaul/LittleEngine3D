#include <iostream>
#include <glad/glad.h>
#include "le3d/log/log.hpp"
#include "le3d/context/context.hpp"
#include "le3d/input/inputImpl.hpp"

namespace le
{
namespace
{
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
	glfwSetErrorCallback(&onError);
	if (!glfwInit())
	{
		logE("Failed to initialise GLFW!");
		return false;
	}
	g_pRenderWindow = glfwCreateWindow(1280, 720, "Test", nullptr, nullptr);
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
	logD("Context destroyed");
}

bool context::isClosing()
{
	return g_pRenderWindow ? glfwWindowShouldClose(g_pRenderWindow) : false;
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
} // namespace le
