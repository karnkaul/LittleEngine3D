#pragma once
#include <bitset>
#include <string>
#include <set>
#include <vector>
#include "le3d/delegate.hpp"
#include "le3d/stdtypes.hpp"

namespace le
{
using OnText = Delegate<char>;
using OnInput = Delegate<s32, s32, s32>;
using OnMouse = Delegate<f64, f64>;
using OnFiledrop = Delegate<std::string_view>;

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

JoyState getJoyState(s32 id);
GamepadState getGamepadState(s32 id);
std::string getClipboard();
} // namespace input
} // namespace le
