#pragma once
#include "le3d/stdtypes.hpp"
#include "le3d/engine/input.hpp"

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
void init(GLFWwindow& window);
void clear();
} // namespace inputImpl
} // namespace le
