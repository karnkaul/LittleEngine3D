#include "le3d/core/log.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/env/env.hpp"
#include "input_impl.hpp"
#if defined(LE3D_USE_GLFW)
#include <GLFW/glfw3.h>
#endif

namespace le
{
namespace
{
std::unique_ptr<inputImpl::Callbacks> g_uCallbacks;
}

inputImpl::Callbacks& inputImpl::callbacks()
{
	if (!g_uCallbacks)
	{
		g_uCallbacks = std::make_unique<Callbacks>();
	}
	return *g_uCallbacks;
}

#if !defined(LE3D_USE_GLFW)

bool inputImpl::init()
{
	LOG_E("FATAL ERROR: Unsupported platform!");
	return false;
}
void inputImpl::setCursorMode(CursorMode) {}
CursorMode inputImpl::cursorMode()
{
	return {};
}
glm::vec2 inputImpl::cursorPos()
{
	return {};
}
void inputImpl::setCursorPos(glm::vec2 const&) {}
JoyState inputImpl::getJoyState(s32)
{
	return {};
}
GamepadState inputImpl::getGamepadState(s32)
{
	return {};
}
f32 inputImpl::triggerToAxis(f32)
{
	return {};
}
std::string inputImpl::getClipboard()
{
	return {};
}
size_t inputImpl::joystickAxesCount(s32)
{
	return {};
}
size_t inputImpl::joysticKButtonsCount(s32)
{
	return {};
}
std::string_view inputImpl::toString(s32)
{
	return {};
}
#else

namespace
{
using namespace inputImpl;

GLFWwindow* g_pWindow = nullptr;

void onKey(GLFWwindow* pWindow, s32 key, s32 /*scancode*/, s32 action, s32 mods)
{
	if (pWindow == g_pWindow)
	{
		callbacks().onInput(Key(key), Action(action), Mods(mods));
	}
}

void onMouseButton(GLFWwindow* pWindow, s32 key, s32 action, s32 mods)
{
	if (pWindow == g_pWindow)
	{
		callbacks().onInput(Key(key + (s32)Key::MOUSE_BUTTON_1), Action(action), Mods(mods));
	}
}

void onText(GLFWwindow* pWindow, u32 codepoint)
{
	if (pWindow == g_pWindow)
	{
		callbacks().onText(static_cast<char>(codepoint));
	}
}

void onMouse(GLFWwindow* pWindow, f64 x, f64 y)
{
	if (pWindow == g_pWindow)
	{
		callbacks().onMouse(x, y);
	}
}

void onScroll(GLFWwindow* pWindow, f64 dx, f64 dy)
{
	if (pWindow == g_pWindow)
	{
		callbacks().onScroll(dx, dy);
	}
}

void onFiledrop(GLFWwindow* pWindow, s32 count, char const** szPaths)
{
	if (pWindow == g_pWindow)
	{
		for (s32 idx = 0; idx < count; ++idx)
		{
			stdfs::path path(szPaths[idx]);
			callbacks().onFiledrop(path);
		}
	}
}

void onFocus(GLFWwindow* pWindow, s32 entered)
{
	if (pWindow == g_pWindow)
	{
		callbacks().onFocus(entered != 0);
	}
}
} // namespace

bool inputImpl::init(GLFWwindow& window)
{
	g_pWindow = &window;
	glfwSetKeyCallback(g_pWindow, &onKey);
	glfwSetCharCallback(g_pWindow, &onText);
	glfwSetCursorPosCallback(g_pWindow, &onMouse);
	glfwSetMouseButtonCallback(g_pWindow, &onMouseButton);
	glfwSetScrollCallback(g_pWindow, &onScroll);
	glfwSetDropCallback(g_pWindow, &onFiledrop);
	glfwSetCursorEnterCallback(g_pWindow, &onFocus);
	return g_pWindow != nullptr;
}

void inputImpl::setCursorMode(CursorMode mode)
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

CursorMode inputImpl::cursorMode()
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

glm::vec2 inputImpl::cursorPos()
{
	if (context::isAlive())
	{
		f64 x, y;
		glfwGetCursorPos(g_pWindow, &x, &y);
		auto size = context::windowSize() * 0.5f;
		return {(f32)x - size.x, size.y - (f32)y};
	}
	return {};
}

void inputImpl::setCursorPos(glm::vec2 const& pos)
{
	glfwSetCursorPos(g_pWindow, pos.x, pos.y);
}

JoyState inputImpl::getJoyState(s32 id)
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

GamepadState inputImpl::getGamepadState(s32 id)
{
	GamepadState ret;
	GLFWgamepadstate glfwState;
	if (glfwJoystickIsGamepad(id) && glfwGetGamepadState(id, &glfwState))
	{
		ret.name = glfwGetGamepadName(id);
		ret.id = id;
		ret.joyState = getJoyState(id);
		return ret;
	}
	return ret;
}

f32 inputImpl::triggerToAxis(f32 triggerValue)
{
	return (triggerValue + 1.0f) * 0.5f;
}

std::string inputImpl::getClipboard()
{
	return glfwGetClipboardString(g_pWindow);
}

void inputImpl::clear()
{
	g_uCallbacks = std::make_unique<Callbacks>();
	g_pWindow = nullptr;
}

size_t inputImpl::joystickAxesCount(s32 id)
{
	s32 max;
	glfwGetJoystickAxes(id, &max);
	return size_t(max);
}

size_t inputImpl::joysticKButtonsCount(s32 id)
{
	s32 max;
	glfwGetJoystickButtons(id, &max);
	return size_t(max);
}

std::string_view inputImpl::toString(s32 key)
{
	return glfwGetKeyName(key, 0);
}
#endif
} // namespace le
