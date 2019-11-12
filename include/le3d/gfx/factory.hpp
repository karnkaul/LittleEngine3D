#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/core/rect2.hpp"
#include "le3d/gfx/colour.hpp"

namespace le
{
struct HVerts
{
	u16 indices = 0;
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

class Transform
{
public:
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::quat m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);
};

struct Texture
{
	GLObj id = 0;
	std::string name;
	std::string type;
};

class Mesh
{
public:
	Transform m_transform;

private:
	std::vector<Vertex> m_vertices;
	std::vector<u32> m_indices;
	std::vector<Texture> m_textures;
	HVerts m_hVerts;

public:
	static Mesh debugCube(f32 side = 0.5f);

public:
	Mesh();
	virtual ~Mesh();
	Mesh(Mesh&&);
	Mesh& operator=(Mesh&&);

public:
	virtual bool setup(std::vector<Vertex> vertices, std::vector<u32> indices, const class Shader* pShader = nullptr);
	virtual void draw(const glm::mat4& v, const glm::mat4& p, class Shader& shader);

public:
	void addTexture(Texture texture);
	void addTextures(std::vector<Texture> textures);

protected:
	void release();
};

namespace gfx
{
Texture genTex(std::string name, std::string type, std::vector<u8> bytes);
void releaseTex(Texture& out_tex);
} // namespace gfx
} // namespace le
