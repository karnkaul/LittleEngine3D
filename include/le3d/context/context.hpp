#pragma once
#include <string>
#include "le3d/stdtypes.hpp"

namespace le
{
namespace context
{
bool create(u16 width, u16 height, std::string_view title);
void destroy();

bool exists();
bool isClosing();
void pollEvents();
void swapBuffers();
}
}
