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

void use(const Shader& shader);
bool setBool(const Shader& shader, std::string_view id, bool bVal);
bool setS32(const Shader& shader, std::string_view id, s32 val);
bool setF32(const Shader& shader, std::string_view id, f32 val);
bool setV2(const Shader& shader, std::string_view id, const glm::vec2& val);
bool setV3(const Shader& shader, std::string_view id, const glm::vec3& val);
bool setV4(const Shader& shader, std::string_view id, const glm::vec4& val);
bool setV4(const Shader& shader, std::string_view id, Colour colour);

void setModelMats(const Shader& shader, const glm::mat4& model, const glm::mat4& normals);
void setViewMats(const Shader& shader, const glm::mat4& view, const glm::mat4& proj);
void setAllMats(const Shader& shader, const glm::mat4& m, const glm::mat4& nm, const glm::mat4& v, const glm::mat4& p);
void setupLights(const Shader& shader, const std::vector<DirLight>& dirLights, const std::vector<PtLight>& pointLights);
} // namespace le::gfx::shading
