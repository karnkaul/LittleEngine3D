#pragma once
#include <glm/glm.hpp>
#include "le3d/core/stdtypes.hpp"
#include "inputTypes.hpp"

namespace le
{
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
	JoyState joyState;
	std::string name;
	s32 id = 0;

	f32 getAxis(PadAxis axis) const;
	bool isPressed(Key button) const;
};

namespace input
{
std::string_view toStr(s32 key);
// Callback parameters: (char utf8)
OnText::Token registerText(OnText::Callback callback);
// Callback parameters: (Key key, Action action, Mods mods)
OnInput::Token registerInput(OnInput::Callback callback);
// Callback parameters: (f64 x, f64 y)
OnMouse::Token registerMouse(OnMouse::Callback callback);
// Callback parameters: (f32 dx, f32 dy)
OnMouse::Token registerScroll(OnMouse::Callback callback);
// Callback parameters: (std::filesystem::path filepath)
OnFiledrop::Token registerFiledrop(OnFiledrop::Callback callback);
// Callback parameters: (bool bInFocus)
OnFocus::Token registerFocus(OnFocus::Callback callback);
// Callback parameters: (s32 x, s32 y)
OnResize::Token registerResize(OnResize::Callback callback);
OnClosed::Token registerClosed(OnClosed::Callback callback);

void setCursorMode(CursorMode mode);
CursorMode cursorMode();
glm::vec2 cursorPos();
void setCursorPos(glm::vec2 const& pos);

JoyState getJoyState(s32 id);
GamepadState getGamepadState(s32 id);
f32 triggerToAxis(f32 triggerValue);
std::string getClipboard();
} // namespace input
} // namespace le
