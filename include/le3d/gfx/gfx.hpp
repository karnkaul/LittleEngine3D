#pragma once
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"

namespace le
{
struct HVerts final
{
	u16 iCount = 0;
	u16 vCount = 0;
	u32 vao = 0;
	u32 vbo = 0;
	u32 ebo = 0;

	HVerts();
	HVerts(HVerts&&);
	HVerts& operator=(HVerts&&);
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
	GLObj glID = 0;
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
	class Shader* pShader = nullptr;
};

namespace gfx
{
namespace gl
{
HVerts genVAO(bool bEBO);
void releaseVAO(HVerts& hVerts);
Texture genTex(std::string name, std::string type, std::vector<u8> bytes);
void releaseTex(std::vector<Texture*> textures);
void bindBuffers(HVerts& hVerts, std::vector<Vertex> vertices, std::vector<u32> indices = {});
HVerts genVertices(std::vector<Vertex> vertices, std::vector<u32> indices = {}, const Shader* pShader = nullptr);

void draw(const HVerts& hVerts, const glm::mat4& m, const glm::mat4& nm, const RenderState& rs, const Shader& s);
} // namespace gl

HVerts newVertices(std::vector<Vertex> vertices, std::vector<u32> indices = {}, const Shader* pShader = nullptr);

namespace tutorial
{
HVerts newLight(const HVerts& hVBO);
}
} // namespace gfx
} // namespace le
