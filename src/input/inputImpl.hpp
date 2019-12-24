#pragma once
#include "le3d/stdtypes.hpp"
#include "le3d/input/input.hpp"

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
};

extern Callbacks g_callbacks;
extern GLFWwindow* g_pRenderWindow;
void init(GLFWwindow& window);
} // namespace inputImpl
} // namespace le
