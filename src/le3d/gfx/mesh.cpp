#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/context/context.hpp"
#include "le3d/context/contextImpl.hpp"
#include "le3d/gfx/mesh.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
s32 Mesh::s_maxTexIdx = 0;

Mesh Mesh::createQuad(f32 side)
{
	f32 s = side * 0.5f;
	f32 points[] = {
		-s, -s, 0.0f, s, -s, 0.0f, s, s, 0.0f, s, s, 0.0f, -s, s, 0.0f, -s, -s, 0.0f,
	};
	f32 norms[] = {
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
	};
	ASSERT(ARR_SIZE(norms) == ARR_SIZE(points), "invalid points/normals array sizes!");
	std::vector<le::Vertex> vertices(ARR_SIZE(points) / 3, le::Vertex());
	ASSERT(ARR_SIZE(points) == vertices.size() * 3, "invalid points / vertices array sizes!");
	glm::vec2 uvs[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}};
	for (size_t idx = 0; idx < vertices.size(); ++idx)
	{
		size_t stride = idx * 3;
		vertices[idx].position = {points[stride], points[stride + 1], points[stride + 2]};
		vertices[idx].normal = {norms[stride], norms[stride + 1], norms[stride + 2]};
		vertices[idx].texCoords = uvs[idx % ARR_SIZE(uvs)];
	}
	Mesh mesh;
	mesh.setupDrawable("debugQuad", std::move(vertices), {});
	return mesh;
}

Mesh Mesh::createCube(f32 side)
{
	f32 s = side * 0.5f;
	f32 points[] = {
		-s, -s, -s, s,	-s, -s, s,	s,	-s, s,	s,	-s, -s, s,	-s, -s, -s, -s, // front

		-s, -s, s,	s,	-s, s,	s,	s,	s,	s,	s,	s,	-s, s,	s,	-s, -s, s, // back

		-s, s,	s,	-s, s,	-s, -s, -s, -s, -s, -s, -s, -s, -s, s,	-s, s,	s, // left

		s,	s,	s,	s,	s,	-s, s,	-s, -s, s,	-s, -s, s,	-s, s,	s,	s,	s, // right

		-s, -s, -s, s,	-s, -s, s,	-s, s,	s,	-s, s,	-s, -s, s,	-s, -s, -s, // down

		-s, s,	-s, s,	s,	-s, s,	s,	s,	s,	s,	s,	-s, s,	s,	-s, s,	-s, // up
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
	ASSERT(ARR_SIZE(norms) == ARR_SIZE(points), "invalid points/normals array sizes!");
	std::vector<le::Vertex> vertices(ARR_SIZE(points) / 3, le::Vertex());
	ASSERT(ARR_SIZE(points) == vertices.size() * 3, "invalid points / vertices array sizes!");
	glm::vec2 uvs[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}};
	for (size_t idx = 0; idx < vertices.size(); ++idx)
	{
		size_t stride = idx * 3;
		vertices[idx].position = {points[stride], points[stride + 1], points[stride + 2]};
		vertices[idx].normal = {norms[stride], norms[stride + 1], norms[stride + 2]};
		vertices[idx].texCoords = uvs[idx % ARR_SIZE(uvs)];
	}
	Mesh mesh;
	mesh.setupDrawable("debugCube", std::move(vertices), {});
	return mesh;
}

void Mesh::glDraw(const glm::mat4& m, const glm::mat4& nm, const RenderState& state, const Shader* pCustomShader)
{
	if (le::context::exists() && m_hVerts.vao > 0)
	{
		auto& shader = pCustomShader ? *pCustomShader : state.shader;
		bool bResetTint = false;
		ASSERT(shader.glID.handle > 0, "shader is null!");
		{
			Lock lock(context::g_glMutex);
			gfx::shading::use(shader);
			gfx::shading::setF32(shader, "material.shininess", m_shininess);
			const auto& v = state.view;
			gfx::shading::setV3(shader, "viewPos", glm::vec3(-v[3][0], -v[3][1], -v[3][2]));
			s32 txID = 0;
			u32 diffuse = 0;
			u32 specular = 0;
			glChk(glBindTexture(GL_TEXTURE_2D, 0));
			auto drawBlankTex = [&](bool bMagenta) {
				if (bMagenta)
				{
					gfx::shading::setV4(shader, "tint", Colour::Magenta);
					bResetTint = true;
				}
				glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
				glChk(glBindTexture(GL_TEXTURE_2D, 1));
			};
#if defined(DEBUGGING)
			if (m_drawFlags.isSet((s32)Flag::Blank) || m_drawFlags.isSet((s32)Flag::BlankMagenta))
			{
				drawBlankTex(m_drawFlags.isSet((s32)Flag::BlankMagenta));
			}
			else
#endif
			{
				if (shader.flags.isSet((s32)gfx::shading::Flag::Untextured))
				{
					if (!shader.flags.isSet((s32)gfx::shading::Flag::Unlit))
					{
						gfx::shading::setV3(shader, "material.ambient", m_untexturedTint.ambient);
						gfx::shading::setV3(shader, "material.diffuse", m_untexturedTint.diffuse);
						gfx::shading::setV3(shader, "material.specular", m_untexturedTint.specular);
					}
				}
				else
				{
					if (m_textures.empty())
					{
						drawBlankTex(true);
					}
				}
				for (const auto& texture : m_textures)
				{
					std::string id = "material.";
					std::string number;
					id += texture.type;
					if (txID > s_maxTexIdx)
					{
						s_maxTexIdx = txID;
					}
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
						if (txID == 0)
						{
							drawBlankTex(true);
						}
						continue;
					}
					id += number;
					if (texture.glID.handle > 0)
					{
						glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
						glBindTexture(GL_TEXTURE_2D, texture.glID.handle);
						gfx::shading::setS32(shader, id, txID++);
					}
					else
					{
						drawBlankTex(true);
					}
				}
			}
		}
		gfx::gl::draw(m_hVerts, m, nm, state, shader);
		for (s32 txID = 0; txID <= s_maxTexIdx; ++txID)
		{
			glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
			glChk(glBindTexture(GL_TEXTURE_2D, 0));
		}
		if (bResetTint)
		{
			gfx::shading::setV4(shader, "tint", Colour::White);
		}
#if defined(DEBUGGING)
		m_drawFlags.flags.reset();
#endif
	}
}
} // namespace le
