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

void use(const HShader& shader);
bool setBool(const HShader& shader, std::string_view id, bool bVal);
bool setS32(const HShader& shader, std::string_view id, s32 val);
bool setF32(const HShader& shader, std::string_view id, f32 val);
bool setV2(const HShader& shader, std::string_view id, const glm::vec2& val);
bool setV3(const HShader& shader, std::string_view id, const glm::vec3& val);
bool setV4(const HShader& shader, std::string_view id, const glm::vec4& val);
bool setV4(const HShader& shader, std::string_view id, Colour colour);

void setModelMats(const HShader& shader, const glm::mat4& model, const glm::mat4& normals);

void bindUBO(const HShader& shader, std::string_view id, const HUBO& ubo);
void setUBO(const HUBO& ubo, s64 offset, s64 size, const void* pData);

template <typename T>
void setUBO(const HUBO& ubo, const T& data);

template <typename T>
void setUBO(const HUBO& ubo, const T& data)
{
	setUBO(ubo, 0, sizeof(data), &data);
}
} // namespace le::gfx::shading
