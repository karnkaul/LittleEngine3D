#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/thirdParty.hpp"

namespace le::gfx::shading
{
enum class Flag
{
	Untextured = 0,
	Unlit,
};

extern const u8 MAX_DIR_LIGHTS;
extern const u8 MAX_POINT_LIGHTS;

void use(Shader shader);
bool setBool(Shader shader, std::string_view id, bool bVal);
bool setS32(Shader shader, std::string_view id, s32 val);
bool setF32(Shader shader, std::string_view id, f32 val);
bool setV2(Shader shader, std::string_view id, const glm::vec2& val);
bool setV3(Shader shader, std::string_view id, const glm::vec3& val);
bool setV4(Shader shader, std::string_view id, const glm::vec4& val);
bool setV4(Shader shader, std::string_view id, Colour colour);

void setupLights(Shader shader, const std::vector<DirLight>& dirLights, const std::vector<PtLight>& pointLights);
} // namespace le::gfx::shading
