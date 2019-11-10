#include "le3d/log/log.hpp"
#include "le3d/input/input.hpp"
#include "inputImpl.hpp"

namespace le
{
namespace
{
OnText g_onText;
GLFWwindow* g_pRenderWindow = nullptr;

void onKey(GLFWwindow* pWindow, s32 key, s32 /*scancode*/, s32 action, s32 mods)
{
	if (pWindow == g_pRenderWindow)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
		{
			glfwSetWindowShouldClose(pWindow, true);
		}
	}
}

void onText(GLFWwindow* pWindow, u32 codepoint) 
{
	if (pWindow == g_pRenderWindow)
	{
		g_onText(static_cast<char>(codepoint));
	}
}

void onMouseMove(GLFWwindow* pWindow, double x, double y) 
{
	logD("Mouse position: %.2f, %.2f", x, y);
}
} // namespace

std::string_view input::toStr(s32 key)
{
	return glfwGetKeyName(key, 0);
}

OnText::Token input::registerText(OnText::Callback callback)
{
	return g_onText.subscribe(callback);
}

void inputImpl::init(GLFWwindow& window)
{
	g_pRenderWindow = &window;
	glfwSetKeyCallback(g_pRenderWindow, &onKey);
	glfwSetCharCallback(g_pRenderWindow, &onText);
	glfwSetCursorPosCallback(g_pRenderWindow, &onMouseMove);
}
} // namespace le
