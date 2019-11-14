#pragma once
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"

namespace le
{
struct HVerts
{
	u16 iCount = 0;
	u16 vCount = 0;
	u32 vao = 0;
	u32 vbo = 0;
	u32 ebo = 0;
};

struct Vertex
{
	glm::vec4 colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec2 texCoords = glm::vec2(0.0f, 0.0f);
};

struct Texture
{
	GLObj id = 0;
	std::string name;
	std::string type;
};

struct Material
{
	std::vector<Texture> textures;
};

struct RenderState
{
	glm::mat4 view;
	glm::mat4 projection;
	class Shader* pShader = nullptr;
};

class Shader;

namespace gfx
{
namespace gl
{
HVerts genVAO(bool bEBO);
void releaseVAO(HVerts& hVerts);
Texture genTex(std::string name, std::string type, std::vector<u8> bytes);
void releaseTex(Texture& out_tex);
void bindBuffers(HVerts& hVerts, std::vector<Vertex> vertices, std::vector<u32> indices = {});
HVerts genBuffers(std::vector<Vertex> vertices, std::vector<u32> indices = {}, const Shader* pShader = nullptr);
} // namespace gl

HVerts newVertices(std::vector<Vertex> vertices, std::vector<u32> indices = {}, const Shader* pShader = nullptr);
} // namespace gfx
} // namespace le
