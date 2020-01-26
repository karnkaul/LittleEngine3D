#pragma once
#include <string>
#include "le3d/core/stdtypes.hpp"
#include "le3d/env/env.hpp"
#include "le3d/engine/input.hpp"

#if defined(LE3D_USE_GLFW)
struct GLFWwindow;
#endif

namespace le
{
namespace inputImpl
{
struct Callbacks
{
	OnText onText;
	OnInput onInput;
	OnMouse onMouse;
	OnMouse onScroll;
	OnFiledrop onFiledrop;
	OnFocus onFocus;
	OnResize onResize;
	OnClosed onClosed;
};

inline Callbacks g_callbacks;
#if !defined(LE3D_USE_GLFW)
bool init();
#else
bool init(GLFWwindow& window);
#endif

void setCursorMode(CursorMode mode);
CursorMode cursorMode();
glm::vec2 cursorPos();
void setCursorPos(glm::vec2 const& pos);

JoyState getJoyState(s32 id);
GamepadState getGamepadState(s32 id);
f32 triggerToAxis(f32 triggerValue);
std::string getClipboard();
void clear();

size_t joystickAxesCount(s32 id);
size_t joysticKButtonsCount(s32 id);

std::string_view toStr(s32 key);
} // namespace inputImpl
} // namespace le
