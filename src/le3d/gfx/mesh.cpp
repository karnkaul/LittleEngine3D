#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/context/context.hpp"
#include "le3d/context/contextImpl.hpp"
#include "le3d/gfx/mesh.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
Mesh Mesh::debugCube(f32 side)
{
	f32 points[] = {
		-side, -side, -side, side,	-side, -side, side,	 side,	-side,
		side,  side,  -side, -side, side,  -side, -side, -side, -side, // front

		-side, -side, side,	 side,	-side, side,  side,	 side,	side,
		side,  side,  side,	 -side, side,  side,  -side, -side, side, // back

		-side, side,  side,	 -side, side,  -side, -side, -side, -side,
		-side, -side, -side, -side, -side, side,  -side, side,	side, // left

		side,  side,  side,	 side,	side,  -side, side,	 -side, -side,
		side,  -side, -side, side,	-side, side,  side,	 side,	side, // right

		-side, -side, -side, side,	-side, -side, side,	 -side, side,
		side,  -side, side,	 -side, -side, side,  -side, -side, -side, // down

		-side, side,  -side, side,	side,  -side, side,	 side,	side,
		side,  side,  side,	 -side, side,  side,  -side, side,	-side, // up
	};
	f32 norms[] = {
		0.0f,  0.0f,  -1.0f, 0.0f,	0.0f,  -1.0f, 0.0f,	 0.0f,	-1.0f,
		0.0f,  0.0f,  -1.0f, 0.0f,	0.0f,  -1.0f, 0.0f,	 0.0f,	-1.0f, // front

		0.0f,  0.0f,  1.0f,	 0.0f,	0.0f,  1.0f,  0.0f,	 0.0f,	1.0f,
		0.0f,  0.0f,  1.0f,	 0.0f,	0.0f,  1.0f,  0.0f,	 0.0f,	1.0f, // back

		-1.0f, 0.0f,  0.0f,	 -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,	0.0f,
		-1.0f, 0.0f,  0.0f,	 -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,	0.0f, // left

		1.0f,  0.0f,  0.0f,	 1.0f,	0.0f,  0.0f,  1.0f,	 0.0f,	0.0f,
		1.0f,  0.0f,  0.0f,	 1.0f,	0.0f,  0.0f,  1.0f,	 0.0f,	0.0f, // right

		0.0f,  -1.0f, 0.0f,	 0.0f,	-1.0f, 0.0f,  0.0f,	 -1.0f, 0.0f,
		0.0f,  -1.0f, 0.0f,	 0.0f,	-1.0f, 0.0f,  0.0f,	 -1.0f, 0.0f, // down

		0.0f,  1.0f,  0.0f,	 0.0f,	1.0f,  0.0f,  0.0f,	 1.0f,	0.0f,
		0.0f,  1.0f,  0.0f,	 0.0f,	1.0f,  0.0f,  0.0f,	 1.0f,	0.0f, // up
	};
	assert(ARR_SIZE(norms) == ARR_SIZE(points));
	std::vector<le::Vertex> vertices(ARR_SIZE(points) / 3, le::Vertex());
	assert(ARR_SIZE(points) == vertices.size() * 3);
	glm::vec2 uvs[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}};
	for (size_t idx = 0; idx < vertices.size(); ++idx)
	{
		size_t stride = idx * 3;
		vertices[idx].position = {points[stride], points[stride + 1], points[stride + 2]};
		vertices[idx].normal = {norms[stride], norms[stride + 1], norms[stride + 2]};
		vertices[idx].texCoords = uvs[idx % ARR_SIZE(uvs)];
	}
	Mesh mesh;
	mesh.setup(std::move(vertices), {});
	return mesh;
}

Mesh::Mesh() = default;

Mesh::~Mesh()
{
	release();
}

Mesh::Mesh(Mesh&&) = default;
Mesh& Mesh::operator=(Mesh&&) = default;

bool Mesh::setup(std::vector<Vertex> vertices, std::vector<u32> indices, const Shader* pShader)
{
	if (le::context::exists())
	{
		release();
		m_hVerts = gfx::newVertices(vertices, indices, pShader);
		return true;
	}
	return false;
}

void Mesh::glDraw(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p, Shader& shader)
{
	if (le::context::exists() && m_hVerts.vao > 0)
	{
		Lock lock(context::g_glMutex);
		shader.use();
		auto temp = glGetUniformLocation(shader.m_program, "model");
		glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(m));
		temp = glGetUniformLocation(shader.m_program, "view");
		glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(v));
		temp = glGetUniformLocation(shader.m_program, "projection");
		glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(p));
		glChk(glBindVertexArray(m_hVerts.vao));
		shader.setS32("use_texture1", m_material.textures.empty() ? 0 : 1);
		s32 txID = 0;
		u32 diffuse = 0;
		for (const auto& texture : m_material.textures)
		{
			std::string id = "material.tex_";
			std::string flag = "material.use_texture";
			flag += std::to_string(txID);
			id += texture.type;
			glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
			if (texture.type == "diffuse")
			{
				id += std::to_string(++diffuse);
			}
			shader.setS32(id, txID++);
			shader.setS32(flag, 1);
			glChk(glBindTexture(GL_TEXTURE_2D, texture.id));
		}
		glChk(glActiveTexture(GL_TEXTURE0));
		if (m_hVerts.ebo > 0)
		{
			glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_hVerts.ebo));
			glChk(glDrawElements(GL_TRIANGLES, m_hVerts.iCount, GL_UNSIGNED_INT, 0));
		}
		else
		{
			glChk(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_hVerts.vCount));
		}
		glBindVertexArray(0);
	}
}

void Mesh::release()
{
	gfx::gl::releaseVAO(m_hVerts);
}
} // namespace le
