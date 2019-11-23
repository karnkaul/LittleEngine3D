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

Mesh::Mesh() = default;
Mesh::Mesh(Mesh&&) = default;
Mesh& Mesh::operator=(Mesh&&) = default;

Mesh::~Mesh()
{
	LOGIF_I(m_hVerts.vao > 0, "-- [%s] %s destroyed", m_name.data(), m_type.data());
	release();
}

Mesh Mesh::createQuad(f32 side)
{
	const f32 s = side * 0.5f;
	const f32 points[] = {
		-s, -s, 0.0f, s, -s, 0.0f, s, s, 0.0f, s, s, 0.0f, -s, s, 0.0f, -s, -s, 0.0f,
	};
	const f32 norms[] = {
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
	};
	ASSERT(ARR_SIZE(norms) == ARR_SIZE(points), "invalid points/normals array sizes!");
	std::vector<le::Vertex> vertices(ARR_SIZE(points) / 3, le::Vertex());
	ASSERT(ARR_SIZE(points) == vertices.size() * 3, "invalid points / vertices array sizes!");
	const glm::vec2 uvs[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}};
	for (size_t idx = 0; idx < vertices.size(); ++idx)
	{
		size_t stride = idx * 3;
		vertices[idx].position = {points[stride], points[stride + 1], points[stride + 2]};
		vertices[idx].normal = {norms[stride], norms[stride + 1], norms[stride + 2]};
		vertices[idx].texCoords = uvs[idx % ARR_SIZE(uvs)];
	}
	Mesh mesh;
	mesh.setupMesh("dQuad", std::move(vertices), {});
	return mesh;
}

Mesh Mesh::createCube(f32 side)
{
	const f32 s = side * 0.5f;
	const f32 points[] = {
		-s, -s, -s, s,	-s, -s, s,	s,	-s, s,	s,	-s, -s, s,	-s, -s, -s, -s, // front

		-s, -s, s,	s,	-s, s,	s,	s,	s,	s,	s,	s,	-s, s,	s,	-s, -s, s, // back

		-s, s,	s,	-s, s,	-s, -s, -s, -s, -s, -s, -s, -s, -s, s,	-s, s,	s, // left

		s,	s,	s,	s,	s,	-s, s,	-s, -s, s,	-s, -s, s,	-s, s,	s,	s,	s, // right

		-s, -s, -s, s,	-s, -s, s,	-s, s,	s,	-s, s,	-s, -s, s,	-s, -s, -s, // down

		-s, s,	-s, s,	s,	-s, s,	s,	s,	s,	s,	s,	-s, s,	s,	-s, s,	-s, // up
	};
	const f32 norms[] = {
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
	const glm::vec2 uvs[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}};
	for (size_t idx = 0; idx < vertices.size(); ++idx)
	{
		size_t stride = idx * 3;
		vertices[idx].position = {points[stride], points[stride + 1], points[stride + 2]};
		vertices[idx].normal = {norms[stride], norms[stride + 1], norms[stride + 2]};
		vertices[idx].texCoords = uvs[idx % ARR_SIZE(uvs)];
	}
	Mesh mesh;
	mesh.setupMesh("dCube", std::move(vertices), {});
	return mesh;
}

Mesh Mesh::create4Pyramid(f32 side)
{
	const f32 s = side * 0.5f;
	const glm::vec3 nF = glm::normalize(glm::cross(glm::vec3(-s, -s, -s), glm::vec3(s, -s, -s)));
	const glm::vec3 nB = glm::normalize(glm::cross(glm::vec3(-s, -s, s), glm::vec3(s, -s, s)));
	const glm::vec3 nL = glm::normalize(glm::cross(glm::vec3(-s, -s, -s), glm::vec3(-s, -s, s)));
	const glm::vec3 nR = glm::normalize(glm::cross(glm::vec3(s, -s, -s), glm::vec3(s, -s, s)));
	const glm::vec3 nD = -g_nUp;
	f32 points[] = {
		-s, -s, -s, s,	-s, -s, 0.0f, s,  0.0f, // front

		-s, -s, s,	s,	-s, s,	0.0f, s,  0.0f, // back

		-s, -s, -s, -s, -s, s,	0.0f, s,  0.0f, // left

		s,	-s, -s, s,	-s, s,	0.0f, s,  0.0f, // right

		-s, -s, -s, s,	-s, -s, s,	  -s, s,	s, -s, s, -s, -s, s, -s, -s, -s, // down
	};
	f32 norms[] = {
		nF.x, nF.y, nF.z, nF.x, nF.y, nF.z, nF.x, nF.y, nF.z, // front

		nB.x, nB.y, nB.z, nB.x, nB.y, nB.z, nB.x, nB.y, nB.z, // back

		nL.x, nL.y, nL.z, nL.x, nL.y, nL.z, nL.x, nL.y, nL.z, // left

		nR.x, nR.y, nR.z, nR.x, nR.y, nR.z, nR.x, nR.y, nR.z, // right

		nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, // down
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
	mesh.setupMesh("dPyramid", std::move(vertices), {});
	return mesh;
}

Mesh Mesh::createTetrahedron(f32 side)
{
	const f32 s = side * 0.5f;
	const f32 t30 = glm::tan(glm::radians(30.0f));
	const f32 r6i = 1.0f / (f32)glm::sqrt(6);
	const glm::vec3 p00(-s, s * -r6i, s * t30);
	const glm::vec3 p01(s, s * -r6i, s * t30);
	const glm::vec3 p02(0.0f, s * -r6i, s * 2.0f * -t30);
	const glm::vec3 p10(0.0f, s * 3 * r6i, 0.0f);
	const glm::vec3 nF = glm::normalize(glm::cross(p01 - p00, p10 - p00));
	const glm::vec3 nL = glm::normalize(glm::cross(p10 - p00, p02 - p00));
	const glm::vec3 nR = glm::normalize(glm::cross(p02 - p01, p10 - p01));
	const glm::vec3 nD = -g_nUp;
	f32 points[] = {
		p00.x, p00.y, p00.z, p10.x, p10.y, p10.z, p01.x, p01.y, p01.z, // front

		p02.x, p02.y, p02.z, p10.x, p10.y, p10.z, p00.x, p00.y, p00.z, // left

		p01.x, p01.y, p01.z, p10.x, p10.y, p10.z, p02.x, p02.y, p02.z, // right

		p00.x, p00.y, p00.z, p01.x, p01.y, p01.z, p02.x, p02.y, p02.z // down
	};
	f32 norms[] = {
		nF.x, nF.y, nF.z, nF.x, nF.y, nF.z, nF.x, nF.y, nF.z, // front

		nL.x, nL.y, nL.z, nL.x, nL.y, nL.z, nL.x, nL.y, nL.z, // left

		nR.x, nR.y, nR.z, nR.x, nR.y, nR.z, nR.x, nR.y, nR.z, // right

		nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, nD.x, nD.y, nD.z, // down
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
	mesh.setupMesh("dTetrahedron", std::move(vertices), {});
	return mesh;
}

bool Mesh::setupMesh(std::string name, std::vector<Vertex> vertices, std::vector<u32> indices, const Shader* pShader)
{
	m_name = std::move(name);
	m_type = Typename(*this);
	if (le::context::exists())
	{
		release();
		m_hVerts = gfx::newVertices(vertices, indices, pShader);
		LOGIF_I(!m_name.empty(), "== [%s] %s set up", m_name.data(), m_type.data());
		return true;
	}
	return false;
}

void Mesh::draw(const Shader& shader) const
{
	if (le::context::exists() && m_hVerts.vao.handle > 0)
	{
		bool bResetTint = false;
		ASSERT(shader.glID.handle > 0, "shader is null!");
		{
			Lock lock(context::g_glMutex);
			gfx::shading::use(shader);
			gfx::shading::setF32(shader, "material.shininess", m_shininess);
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
				if (!shader.flags.isSet((s32)gfx::shading::Flag::Untextured))
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
		gfx::gl::draw(m_hVerts);
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

const HVerts& Mesh::VAO() const
{
	return m_hVerts;
}

void Mesh::release()
{
	gfx::gl::releaseVAO(m_hVerts);
}
} // namespace le
