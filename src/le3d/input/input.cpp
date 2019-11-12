#include <algorithm>
#include <unordered_map>
#include "le3d/log/log.hpp"
#include "le3d/input/input.hpp"
#include "inputImpl.hpp"

namespace le
{
namespace
{
struct Callbacks
{
	OnText onText;
	OnInput onInput;
	OnMouse onMouse;
	OnMouse onScroll;
	OnFiledrop onFiledrop;
};

Callbacks g_callbacks;
GLFWwindow* g_pRenderWindow = nullptr;

void onKey(GLFWwindow* pWindow, s32 key, s32 /*scancode*/, s32 action, s32 mods)
{
	if (pWindow == g_pRenderWindow)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
		{
			glfwSetWindowShouldClose(pWindow, true);
		}

		g_callbacks.onInput(key, action, mods);
	}
}

void onMouseButton(GLFWwindow* pWindow, s32 key, s32 action, s32 mods)
{
	if (pWindow == g_pRenderWindow)
	{
		g_callbacks.onInput(key, action, mods);
	}
}

void onText(GLFWwindow* pWindow, u32 codepoint)
{
	if (pWindow == g_pRenderWindow)
	{
		g_callbacks.onText(static_cast<char>(codepoint));
	}
}

void onMouse(GLFWwindow* pWindow, f64 x, f64 y)
{
	if (pWindow == g_pRenderWindow)
	{
		g_callbacks.onMouse(x, y);
	}
}

void onScroll(GLFWwindow* pWindow, f64 dx, f64 dy)
{
	if (pWindow == g_pRenderWindow)
	{
		g_callbacks.onScroll(dx, dy);
	}
}

void onFiledrop(GLFWwindow* pWindow, s32 count, const char** szPaths)
{
	if (pWindow == g_pRenderWindow)
	{
		for (s32 idx = 0; idx < count; ++idx)
		{
			std::string path(szPaths[idx]);
			g_callbacks.onFiledrop(path);
		}
	}
}
} // namespace

std::string_view input::toStr(s32 key)
{
	return glfwGetKeyName(key, 0);
}

OnText::Token input::registerText(OnText::Callback callback)
{
	return g_callbacks.onText.subscribe(callback);
}

OnInput::Token input::registerInput(OnInput::Callback callback)
{
	return g_callbacks.onInput.subscribe(callback);
}

OnMouse::Token input::registerMouse(OnMouse::Callback callback)
{
	return g_callbacks.onMouse.subscribe(callback);
}

OnMouse::Token input::registerScroll(OnMouse::Callback callback)
{
	return g_callbacks.onScroll.subscribe(callback);
}

OnFiledrop::Token input::registerFiledrop(OnFiledrop::Callback callback)
{
	return g_callbacks.onFiledrop.subscribe(callback);
}

JoyState input::getJoyState(s32 id)
{
	JoyState ret;
	if (glfwJoystickPresent(id))
	{
		ret.id = id;
		s32 count;
		const auto axes = glfwGetJoystickAxes(id, &count);
		ret.axes.reserve(toIdx(count));
		for (s32 idx = 0; idx < count; ++idx)
		{
			ret.axes.push_back(axes[idx]);
		}
		const auto buttons = glfwGetJoystickButtons(id, &count);
		ret.buttons.reserve(toIdx(count));
		for (s32 idx = 0; idx < count; ++idx)
		{
			ret.buttons.push_back(buttons[idx]);
		}
		const auto szName = glfwGetJoystickName(id);
		if (szName)
		{
			ret.name = szName;
		}
	}
	return ret;
}

GamepadState input::getGamepadState(s32 id)
{
	GamepadState ret;
	if (glfwJoystickIsGamepad(id) && glfwGetGamepadState(id, &ret.glfwState))
	{
		ret.name = glfwGetGamepadName(id);
		ret.id = id;
		return ret;
	}
	return ret;
}

std::string input::getClipboard()
{
	return glfwGetClipboardString(g_pRenderWindow);
}

void inputImpl::init(GLFWwindow& window)
{
	g_pRenderWindow = &window;
	glfwSetKeyCallback(g_pRenderWindow, &onKey);
	glfwSetCharCallback(g_pRenderWindow, &onText);
	glfwSetCursorPosCallback(g_pRenderWindow, &onMouse);
	glfwSetMouseButtonCallback(g_pRenderWindow, &onMouseButton);
	glfwSetScrollCallback(g_pRenderWindow, &onScroll);
	glfwSetDropCallback(g_pRenderWindow, &onFiledrop);
}
} // namespace le
