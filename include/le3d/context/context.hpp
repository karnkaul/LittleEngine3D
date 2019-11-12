#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/core/vector2.hpp"
#include "le3d/gfx/colour.hpp"

namespace le
{
namespace context
{
bool create(u16 width, u16 height, std::string_view title);
void destroy();

bool exists();
bool isClosing();
void clearFlags(Colour colour = Colour::Black, u32 flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
void pollEvents();
void swapBuffers();

Vector2 size();
Vector2 project(Vector2 nPos, Vector2 space);
Vector2 projectScreen(Vector2 nPos);
Vector2 worldToScreen(Vector2 world);
} // namespace context
} // namespace le
