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
		m_indices = std::move(indices);
		m_vertices = std::move(vertices);
		m_hVerts = gfx::genVAO(!m_indices.empty());

		Lock lock(context::g_glMutex);
		glChk(glBindVertexArray(m_hVerts.vao));
		glChk(glBindBuffer(GL_ARRAY_BUFFER, m_hVerts.vbo));
		glChk(glBufferData(GL_ARRAY_BUFFER, (s64)m_vertices.size() * (s64)sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW));
		if (!m_indices.empty())
		{
			glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_hVerts.ebo));
			glChk(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (s64)m_indices.size() * (s64)sizeof(u32), m_indices.data(), GL_STATIC_DRAW));
		}

		const auto stride = sizeof(Vertex);
		// Colour
		GLint loc = 0;
		if (pShader)
		{
			loc = glGetAttribLocation(pShader->m_program, "aColour");
		}
		if (loc >= 0)
		{
			glChk(glVertexAttribPointer((GLObj)loc, 4, GL_FLOAT, GL_FALSE, stride, (void*)(offsetof(Vertex, colour))));
			glChk(glEnableVertexAttribArray((GLObj)loc));
		}

		// Position
		loc = 1;
		if (pShader)
		{
			loc = glGetAttribLocation(pShader->m_program, "aPosition");
		}
		if (loc >= 0)
		{
			glChk(glVertexAttribPointer((GLObj)loc, 3, GL_FLOAT, GL_FALSE, stride, (void*)(offsetof(Vertex, position))));
			glChk(glEnableVertexAttribArray((GLObj)loc));
		}

		// Normal
		loc = 2;
		if (pShader)
		{
			loc = glGetAttribLocation(pShader->m_program, "aNormal");
		}
		if (loc >= 0)
		{
			glChk(glVertexAttribPointer((GLObj)loc, 3, GL_FLOAT, GL_FALSE, stride, (void*)(offsetof(Vertex, normal))));
			glChk(glEnableVertexAttribArray((GLObj)loc));
		}

		// Tex coord
		loc = 3;
		if (pShader)
		{
			loc = glGetAttribLocation(pShader->m_program, "aTexCoord");
		}
		if (loc >= 0)
		{
			glChk(glVertexAttribPointer((GLObj)loc, 2, GL_FLOAT, GL_FALSE, stride, (void*)(offsetof(Vertex, texCoords))));
			glChk(glEnableVertexAttribArray((GLObj)loc));
		}

		return true;
	}
	return false;
}

void Mesh::draw(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p, Shader& shader)
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
			glChk(glDrawElements(GL_TRIANGLES, m_hVerts.indices, GL_UNSIGNED_INT, 0));
		}
		else
		{
			glChk(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices.size()));
		}
		glBindVertexArray(0);
	}
}

void Mesh::release()
{
	gfx::releaseVAO(m_hVerts);
}
} // namespace le
