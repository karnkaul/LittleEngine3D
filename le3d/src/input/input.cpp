#include <algorithm>
#include <unordered_map>
#include "le3d/context/context.hpp"
#include "le3d/core/log.hpp"
#include "le3d/input/input.hpp"
#include "inputImpl.hpp"

namespace le
{
using namespace inputImpl;

namespace
{
GLFWwindow* g_pWindow = nullptr;
}

namespace
{
void onKey(GLFWwindow* pWindow, s32 key, s32 /*scancode*/, s32 action, s32 mods)
{
	if (pWindow == g_pWindow)
	{
		g_callbacks.onInput(key, action, mods);
	}
}

void onMouseButton(GLFWwindow* pWindow, s32 key, s32 action, s32 mods)
{
	if (pWindow == g_pWindow)
	{
		g_callbacks.onInput(key, action, mods);
	}
}

void onText(GLFWwindow* pWindow, u32 codepoint)
{
	if (pWindow == g_pWindow)
	{
		g_callbacks.onText(static_cast<char>(codepoint));
	}
}

void onMouse(GLFWwindow* pWindow, f64 x, f64 y)
{
	if (pWindow == g_pWindow)
	{
		g_callbacks.onMouse(x, y);
	}
}

void onScroll(GLFWwindow* pWindow, f64 dx, f64 dy)
{
	if (pWindow == g_pWindow)
	{
		g_callbacks.onScroll(dx, dy);
	}
}

void onFiledrop(GLFWwindow* pWindow, s32 count, char const** szPaths)
{
	if (pWindow == g_pWindow)
	{
		for (s32 idx = 0; idx < count; ++idx)
		{
			stdfs::path path(szPaths[idx]);
			g_callbacks.onFiledrop(path);
		}
	}
}

void onFocus(GLFWwindow* pWindow, s32 entered)
{
	if (pWindow == g_pWindow)
	{
		g_callbacks.onFocus(entered != 0);
	}
}
} // namespace

f32 GamepadState::getAxis(s32 axis) const
{
	s32 max;
	glfwGetJoystickAxes(id, &max);
	return axis >= 0 && axis < max ? glfwState.axes[axis] : 0.0f;
}

bool GamepadState::isPressed(s32 button) const
{
	s32 max;
	glfwGetJoystickButtons(id, &max);
	return button >= 0 && button <= max ? glfwState.buttons[button] : false;
}

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

OnFocus::Token input::registerFocus(OnFocus::Callback callback)
{
	return g_callbacks.onFocus.subscribe(callback);
}

OnResize::Token input::registerResize(OnResize::Callback callback)
{
	return g_callbacks.onResize.subscribe(callback);
}

OnClosed::Token input::registerClosed(OnClosed::Callback callback)
{
	return g_callbacks.onClosed.subscribe(callback);
}

void input::setCursorMode(CursorMode mode)
{
	if (context::isAlive())
	{
		s32 val;
		switch (mode)
		{
		case CursorMode::Default:
			val = GLFW_CURSOR_NORMAL;
			break;

		case CursorMode::Hidden:
			val = GLFW_CURSOR_HIDDEN;
			break;

		case CursorMode::Disabled:
			val = GLFW_CURSOR_DISABLED;
			break;

		default:
			val = glfwGetInputMode(g_pWindow, GLFW_CURSOR);
			break;
		}
		glfwSetInputMode(g_pWindow, GLFW_CURSOR, val);
	}
}

CursorMode input::cursorMode()
{
	CursorMode ret = CursorMode::Default;
	if (context::isAlive())
	{
		s32 val = glfwGetInputMode(g_pWindow, GLFW_CURSOR);
		switch (val)
		{
		case GLFW_CURSOR_NORMAL:
			ret = CursorMode::Default;
			break;

		case GLFW_CURSOR_HIDDEN:
			ret = CursorMode::Hidden;
			break;

		case GLFW_CURSOR_DISABLED:
			ret = CursorMode::Disabled;
			break;

		default:
			break;
		}
	}
	return ret;
}

glm::vec2 input::cursorPos()
{
	if (context::isAlive())
	{
		f64 x, y;
		glfwGetCursorPos(g_pWindow, &x, &y);
		return {(f32)x, (f32)y};
	}
	return {};
}

void input::setCursorPos(glm::vec2 pos)
{
	glfwSetCursorPos(g_pWindow, pos.x, pos.y);
}

JoyState input::getJoyState(s32 id)
{
	JoyState ret;
	if (glfwJoystickPresent(id))
	{
		ret.id = id;
		s32 count;
		auto const axes = glfwGetJoystickAxes(id, &count);
		ret.axes.reserve((size_t)count);
		for (s32 idx = 0; idx < count; ++idx)
		{
			ret.axes.push_back(axes[idx]);
		}
		auto const buttons = glfwGetJoystickButtons(id, &count);
		ret.buttons.reserve((size_t)count);
		for (s32 idx = 0; idx < count; ++idx)
		{
			ret.buttons.push_back(buttons[idx]);
		}
		auto const szName = glfwGetJoystickName(id);
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

f32 input::triggerToAxis(f32 triggerValue)
{
	return (triggerValue + 1.0f) * 0.5f;
}

std::string input::getClipboard()
{
	return glfwGetClipboardString(g_pWindow);
}

void inputImpl::init(GLFWwindow& window)
{
	g_pWindow = &window;
	glfwSetKeyCallback(g_pWindow, &onKey);
	glfwSetCharCallback(g_pWindow, &onText);
	glfwSetCursorPosCallback(g_pWindow, &onMouse);
	glfwSetMouseButtonCallback(g_pWindow, &onMouseButton);
	glfwSetScrollCallback(g_pWindow, &onScroll);
	glfwSetDropCallback(g_pWindow, &onFiledrop);
	glfwSetCursorEnterCallback(g_pWindow, &onFocus);
}

void inputImpl::clear()
{
	g_callbacks = Callbacks();
	g_pWindow = nullptr;
}
} // namespace le
