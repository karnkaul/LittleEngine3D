#pragma once
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"

namespace le
{
#define glChk(expr)                           \
	do                                        \
	{                                         \
		expr;                                 \
		le::glCheckError(__FILE__, __LINE__); \
	} while (0);

constexpr glm::vec3 g_nUp = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 g_nRight = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 g_nFront = glm::vec3(0.0f, 0.0f, 1.0f);
constexpr glm::vec3 g_nForward = -g_nFront;

s32 glCheckError(const char* szFile, s32 line);
} // namespace le
