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
		-side, -side, -side, side,  -side, -side, side,  side,  -side,
		side,  side,  -side, -side, side,  -side, -side, -side, -side, // front

		-side, -side, side,  side,  -side, side,  side,  side,  side,
		side,  side,  side,  -side, side,  side,  -side, -side, side, // back

		-side, side,  side,  -side, side,  -side, -side, -side, -side,
		-side, -side, -side, -side, -side, side,  -side, side,  side, // left

		side,  side,  side,  side,  side,  -side, side,  -side, -side,
		side,  -side, -side, side,  -side, side,  side,  side,  side, // right

		-side, -side, -side, side,  -side, -side, side,  -side, side,
		side,  -side, side,  -side, -side, side,  -side, -side, -side, // down

		-side, side,  -side, side,  side,  -side, side,  side,  side,
		side,  side,  side,  -side, side,  side,  -side, side,  -side, // up
	};
	f32 norms[] = {
		0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f,
		0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, // front

		0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, // back

		-1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,
		-1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f, // left

		1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, // right

		0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,
		0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f, // down

		0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // up
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

const HVerts& Mesh::VAO() const
{
	return m_hVerts;
}

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

void Mesh::glDraw(const glm::mat4& m, const glm::mat4& nm, const RenderState& state, const Shader* pCustomShader)
{
	if (le::context::exists() && m_hVerts.vao > 0)
	{
		auto* pShader = pCustomShader ? pCustomShader : state.pShader;
		bool bResetTint = false;
		assert(pShader && "shader is null!");
		{
			Lock lock(context::g_glMutex);
			pShader->use();
			const auto& v = state.view;
			pShader->setV3("viewPos", glm::vec3(-v[3][0], -v[3][1], -v[3][2]));
			s32 txID = 0;
			u32 diffuse = 0;
			u32 specular = 0;
#if defined(DEBUGGING)
			if (m_renderFlags.isSet((s32)Flag::Blank) || m_renderFlags.isSet((s32)Flag::BlankMagenta))
			{
				if (m_renderFlags.isSet((s32)Flag::BlankMagenta))
				{
					pShader->setV4("tint", Colour::Magenta);
					bResetTint = true;
				}
				glChk(glActiveTexture(GL_TEXTURE0));
				glChk(glBindTexture(GL_TEXTURE_2D, 1));
			}
			else
#endif
			{
				if (!pShader->m_flags.isSet((s32)Shader::Flag::Untextured) && m_textures.empty())
				{
					pShader->setV4("tint", Colour::Magenta);
					glChk(glActiveTexture(GL_TEXTURE0));
					glChk(glBindTexture(GL_TEXTURE_2D, 1));
					bResetTint = true;
				}
				for (const auto& texture : m_textures)
				{
					std::string id = "material.";
					std::string number;
					id += texture.type;
					glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
					if (texture.type == "diffuse")
					{
						number = std::to_string(++diffuse);
					}
					else if (texture.type == "specular")
					{
						number = std::to_string(++specular);
					}
					else
					{
						continue;
					}
					id += number;
					pShader->setS32(id, txID++);
					glChk(glBindTexture(GL_TEXTURE_2D, texture.id));
				}
			}
		}
		gfx::draw(m_hVerts, m, nm, state, *pShader);
		glChk(glActiveTexture(GL_TEXTURE0));
		glChk(glBindTexture(GL_TEXTURE_2D, 0));
		if (bResetTint)
		{
			pShader->setV4("tint", Colour::White);
		}
#if defined(DEBUGGING)
		m_renderFlags.flags.reset();
#endif
	}
}

void Mesh::release()
{
	gfx::gl::releaseVAO(m_hVerts);
}
} // namespace le
