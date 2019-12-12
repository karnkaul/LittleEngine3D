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
constexpr glm::quat g_qIdentity = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

s32 glCheckError(const char* szFile, s32 line);

constexpr glm::ivec4 viewportDxDyXY(const glm::vec2& vpSize, f32 spaceAR)
{
	const f32 vpAR = vpSize.x / vpSize.y;
	spaceAR = spaceAR <= 0.0f ? vpAR : spaceAR;
	const f32 uiW = vpAR > spaceAR ? vpSize.x * spaceAR / vpAR : vpSize.x;
	const f32 uiH = vpAR < spaceAR ? vpSize.y * vpAR / spaceAR : vpSize.y;
	return {(s32)((vpSize.x - uiW) * 0.5f), (s32)((vpSize.y - uiH) * 0.5f), (s32)uiW, (s32)uiH};
}
} // namespace le
