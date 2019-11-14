#pragma once
#include <vector>
#include "le3d/stdtypes.hpp"
#include "le3d/core/transform.hpp"
#include "le3d/gfx/factory.hpp"

namespace le
{
class Mesh
{
public:
	Material m_material;

private:
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
	virtual void glDraw(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p, class Shader& shader);

protected:
	void release();
};
} // namespace le
