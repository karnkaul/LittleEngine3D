#pragma once
#include "le3d/thirdParty.hpp"
#include "le3d/stdtypes.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/core/tZero.hpp"

namespace le
{
using GLObj = TZero<u32>;

struct HVerts final
{
	GLObj vao;
	GLObj vbo;
	GLObj ebo;
	u16 iCount = 0;
	u16 vCount = 0;
};

struct Vertex final
{
	glm::vec4 colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec2 texCoords = glm::vec2(0.0f, 0.0f);
};

struct Texture final
{
	std::string id;
	std::string type;
	GLObj glID;
};

struct Shader final
{
	static const size_t MAX_FLAGS = 8;
	std::string id;
	Flags<MAX_FLAGS> flags;
	GLObj glID;
};

struct LitTint final
{
	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);
};

struct LightData final
{
	glm::vec3 ambient = glm::vec3(0.2f);
	glm::vec3 diffuse = glm::vec3(0.5f);
	glm::vec3 specular = glm::vec3(1.0f);
};

struct DirLight final
{
	LightData light;
	glm::vec3 direction = glm::vec3(-1.0f, -1.0f, 1.0f);
};

struct PtLight final
{
	LightData light;
	glm::vec3 position = glm::vec3(0.0f);
	f32 constant = 1.0f;
	f32 linear = 0.09f;
	f32 quadratic = 0.032f;
};

struct RenderState final
{
	glm::mat4 view;
	glm::mat4 projection;
	std::vector<PtLight> pointLights;
	std::vector<DirLight> dirLights;
};
} // namespace le
