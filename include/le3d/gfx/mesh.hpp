#pragma once
#include <vector>
#include "le3d/stdtypes.hpp"
#include "le3d/core/transform.hpp"
#include "le3d/gfx/factory.hpp"

namespace le
{
class Mesh
{
private:
	std::vector<Vertex> m_vertices;
	std::vector<Texture> m_textures;
	std::vector<u32> m_indices;
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
	virtual void draw(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p, class Shader& shader);

public:
	void addTexture(Texture texture);
	void addTextures(std::vector<Texture> textures);

protected:
	void release();
};
} // namespace le
