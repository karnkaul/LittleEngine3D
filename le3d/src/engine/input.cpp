#include <algorithm>
#include <unordered_map>
#include "le3d/core/log.hpp"
#include "le3d/engine/input.hpp"
#include "input_impl.hpp"

namespace le
{
using namespace inputImpl;

f32 GamepadState::getAxis(PadAxis axis) const
{
	size_t idx = size_t(axis);
	return idx < inputImpl::joystickAxesCount(id) ? joyState.axes[idx] : 0.0f;
}

bool GamepadState::isPressed(Key button) const
{
	size_t idx = (size_t)button - (size_t)Key::GAMEPAD_BUTTON_A;
	return idx < inputImpl::joysticKButtonsCount(id) ? joyState.buttons[idx] : false;
}

std::string_view input::toString(s32 key)
{
	return inputImpl::toString(key);
}

OnText::Token input::registerText(OnText::Callback callback)
{
	return inputImpl::callbacks().onText.subscribe(callback);
}

OnInput::Token input::registerInput(OnInput::Callback callback)
{
	return inputImpl::callbacks().onInput.subscribe(callback);
}

OnMouse::Token input::registerMouse(OnMouse::Callback callback)
{
	return inputImpl::callbacks().onMouse.subscribe(callback);
}

OnMouse::Token input::registerScroll(OnMouse::Callback callback)
{
	return inputImpl::callbacks().onScroll.subscribe(callback);
}

OnFiledrop::Token input::registerFiledrop(OnFiledrop::Callback callback)
{
	return inputImpl::callbacks().onFiledrop.subscribe(callback);
}

OnFocus::Token input::registerFocus(OnFocus::Callback callback)
{
	return inputImpl::callbacks().onFocus.subscribe(callback);
}

OnResize::Token input::registerResize(OnResize::Callback callback)
{
	return inputImpl::callbacks().onResize.subscribe(callback);
}

OnClosed::Token input::registerClosed(OnClosed::Callback callback)
{
	return inputImpl::callbacks().onClosed.subscribe(callback);
}

void input::setCursorMode(CursorMode mode)
{
	inputImpl::setCursorMode(mode);
	return;
}

CursorMode input::cursorMode()
{
	return inputImpl::cursorMode();
}

glm::vec2 input::cursorPos()
{
	return inputImpl::cursorPos();
}

void input::setCursorPos(glm::vec2 const& pos)
{
	inputImpl::setCursorPos(pos);
	return;
}

JoyState input::getJoyState(s32 id)
{
	return inputImpl::getJoyState(id);
}

GamepadState input::getGamepadState(s32 id)
{
	return inputImpl::getGamepadState(id);
}

f32 input::triggerToAxis(f32 triggerValue)
{
	return (triggerValue + 1.0f) * 0.5f;
}

std::string input::getClipboard()
{
	return inputImpl::getClipboard();
}
} // namespace le
