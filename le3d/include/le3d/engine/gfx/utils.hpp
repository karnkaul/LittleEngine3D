#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "le3d/core/stdtypes.hpp"
#include "le3d/core/rect2.hpp"

namespace le
{
#define glChk(expr)                                \
	do                                             \
	{                                              \
		expr;                                      \
		le::gfx::glCheckError(__FILE__, __LINE__); \
	} while (0);

constexpr glm::vec3 g_nUp = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 g_nRight = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 g_nFront = glm::vec3(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 g_nForward = -g_nFront;
constexpr glm::quat g_qIdentity = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

namespace gfx
{
s32 glCheckError(char const* szFile, s32 line);

void setViewport(Rect2 const& view);
void setViewport(s32 x, s32 dx, s32 y, s32 dy);

Rect2 cropView(Rect2 const& view, f32 spaceAspect);
Rect2 const& view();
void setView(Rect2 const& view);
} // namespace gfx
} // namespace le
