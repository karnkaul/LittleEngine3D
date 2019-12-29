#pragma once
#include <string>
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"

namespace le
{
namespace context
{
extern bool g_bVSYNC;

bool create(u16 width, u16 height, std::string_view title);
void destroy();

bool exists();
bool isClosing();
void clearFlags(u32 flags, Colour colour = Colour::Black);
void pollEvents();
void swapBuffers();

f32 nativeAR();
glm::vec2 size();
glm::vec2 project(glm::vec2 nPos, glm::vec2 space);
glm::vec2 projectScreen(glm::vec2 nPos);
glm::vec2 worldToScreen(glm::vec2 world);
} // namespace context
} // namespace le
