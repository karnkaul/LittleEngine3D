#pragma once
#include <string>
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/env/env.hpp"

namespace le
{
namespace context
{
extern bool g_bVSYNC;

struct LogOpts
{
	std::string_view filename = "debug.log";
	env::Dir dir = env::Dir::Working;
	bool bLogToFile = true;
};

bool create(u16 width, u16 height, std::string_view title, LogOpts logOpts = {});
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
