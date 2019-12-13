#pragma once
#include <bitset>
#include <string>
#include <set>
#include <vector>
#include "le3d/delegate.hpp"
#include "le3d/stdtypes.hpp"
#include "le3d/thirdParty.hpp"

namespace le
{
using OnText = Delegate<char>;
using OnInput = Delegate<s32, s32, s32>;
using OnMouse = Delegate<f64, f64>;
using OnFocus = Delegate<bool>;
using OnFiledrop = Delegate<std::string_view>;
using OnResize = Delegate<s32, s32>;

enum class CursorMode
{
	Default = 0,
	Hidden,
	Disabled,
};

struct JoyState
{
	std::string name;
	s32 id = 0;
	std::vector<f32> axes;
	std::vector<u8> buttons;
};

struct GamepadState
{
	GLFWgamepadstate glfwState;
	std::string name;
	s32 id = 0;

	f32 getAxis(s32 axis) const;
	bool isPressed(s32 button) const;
};

namespace input
{
std::string_view toStr(s32 key);
// Callback parameters: (char utf8)
OnText::Token registerText(OnText::Callback callback);
// Callback parameters: (s32 key, s32 action, s32 mods)
OnInput::Token registerInput(OnInput::Callback callback);
// Callback parameters: (f64 x, f64 y)
OnMouse::Token registerMouse(OnMouse::Callback callback);
// Callback parameters: (f32 dx, f32 dy)
OnMouse::Token registerScroll(OnMouse::Callback callback);
// Callback parameters: (std::string filepath)
OnFiledrop::Token registerFiledrop(OnFiledrop::Callback callback);
// Callback parameters: (bool bInFocus)
OnFocus::Token registerFocus(OnFocus::Callback callback);
// Callback parameters: (s32 x, s32 y)
OnResize::Token registerResize(OnResize::Callback callback);

void setCursorMode(CursorMode mode);
CursorMode cursorMode();
glm::vec2 cursorPos();
void setCursorPos(glm::vec2 pos);

JoyState getJoyState(s32 id);
GamepadState getGamepadState(s32 id);
f32 triggerToAxis(f32 triggerValue);
std::string getClipboard();
} // namespace input
} // namespace le
