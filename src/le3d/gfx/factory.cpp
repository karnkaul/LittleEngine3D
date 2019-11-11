#include <assert.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include "le3d/context/context.hpp"
#include "le3d/gfx/factory.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/log/log.hpp"

namespace le
{
Primitive::Primitive() = default;
Primitive::~Primitive()
{
	release();
}
Primitive::Primitive(Primitive&&) = default;
Primitive& Primitive::operator=(Primitive&&) = default;

void Primitive::setShader(Shader& shader)
{
	m_pShader = &shader;
}

void Primitive::provisionQuad(Rect2 model, Rect2 uvs, Colour colour)
{
	release();
	m_verts = gfx::genQuad(model, uvs, colour);
}

void Primitive::draw(const glm::mat4& view, const glm::mat4& proj)
{
	if (le::context::exists() && m_verts.vao > 0)
	{
		if (m_pShader)
		{
			m_pShader->use();
			auto temp = glGetUniformLocation(m_pShader->program(), "uModel");
			glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(m_local));
			temp = glGetUniformLocation(m_pShader->program(), "uView");
			glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(view));
			temp = glGetUniformLocation(m_pShader->program(), "uProjection");
			glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(proj));
		}
		glBindVertexArray(m_verts.vao);
		glChk();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_verts.ebo);
		glChk();
		glDrawElements(GL_TRIANGLES, m_verts.indices, GL_UNSIGNED_INT, 0);
		glChk();
	}
}

void Primitive::release()
{
	if (le::context::exists() && m_verts.vao > 0)
	{
		glDeleteVertexArrays(1, &m_verts.vao);
		glChk();
		glDeleteBuffers(1, &m_verts.vbo);
		glChk();
		glDeleteBuffers(1, &m_verts.ebo);
		glChk();
	}
	m_verts.vao = m_verts.vbo = m_verts.ebo = 0;
	m_verts.indices = 0;
}


std::vector<f32> gfx::buildVertices(std::vector<Vector2> points, std::vector<Colour> colours, std::vector<Vector2> STs)
{
	assert(points.size() == colours.size() && points.size() == STs.size() && "array size mismatch!");
	size_t count = points.size();
	std::vector<f32> ret;
	ret.reserve(count * 9);
	for (size_t i = 0; i < count; ++i)
	{
		ret.push_back(points[i].x.toF32());
		ret.push_back(points[i].y.toF32());
		ret.push_back(0.0f);
		ret.push_back(colours[i].r.toF32());
		ret.push_back(colours[i].g.toF32());
		ret.push_back(colours[i].b.toF32());
		ret.push_back(colours[i].a.toF32());
		ret.push_back(STs[i].x.toF32());
		ret.push_back(STs[i].y.toF32());
	}
	return ret;
}

HVerts gfx::genVerts(std::vector<Vector2> points, std::vector<Colour> colours, std::vector<Vector2> STs)
{
	HVerts v;
	std::vector<f32> verts = buildVertices(std::move(points), std::move(colours), std::move(STs));
	const auto stride = 9 * sizeof(float);
	glGenVertexArrays(1, &v.vao);
	glChk();
	glGenBuffers(1, &v.vbo);
	glChk();
	glBindVertexArray(v.vao);
	glChk();
	glBindBuffer(GL_ARRAY_BUFFER, v.vbo);
	glChk();
	glGenBuffers(1, &v.ebo);
	glChk();

	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(SIZEOF_VEC(verts)), verts.data(), GL_STATIC_DRAW);
	glChk();

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glChk();

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glChk();

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glChk();

	return v;
}

HVerts gfx::genQuad(Rect2 model, Rect2 uvs, Colour c)
{
	std::vector<Vector2> points = {model.topRight(), model.bottomRight(), model.bottomLeft(), model.topLeft()};
	std::vector<Colour> colours(4, c);
	std::vector<Vector2> STs = {uvs.topRight(), uvs.bottomRight(), uvs.bottomLeft(), uvs.topLeft()};
	HVerts v;
	const u32 indices[] = {0, 1, 3,

						   1, 2, 3};
	v = genVerts(std::move(points), std::move(colours), std::move(STs));
	v.indices = ARR_SIZE(indices);
	glGenBuffers(1, &v.ebo);
	glChk();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, v.ebo);
	glChk();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glChk();
	return v;
}

void gfx::releaseVerts(HVerts verts) 
{
	if (le::context::exists() && verts.vao > 0)
	{
		glDeleteVertexArrays(1, &verts.vao);
		glChk();
		glDeleteBuffers(1, &verts.vbo);
		glChk();
		glDeleteBuffers(1, &verts.ebo);
		glChk();
	}
}

GLObj gfx::genTex(std::vector<u8> bytes)
{
	s32 w, h, ch;
	GLObj hTex = 0;
	stbi_set_flip_vertically_on_load(1);
	auto pData = stbi_load_from_memory(bytes.data(), toS32(bytes.size()), &w, &h, &ch, 0);
	//auto pData = stbi_load(path.data(), &w, &h, &ch, 0);
	if (pData)
	{
		glGenTextures(1, &hTex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, ch == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pData);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		logE("Failed to load texture!");
	}
	stbi_image_free(pData);
	return hTex;
}

void gfx::releaseTex(GLObj& out_hTex)
{
	const GLuint glTex[] = {out_hTex};
	glDeleteTextures(1, glTex);
	out_hTex = 0;
}
} // namespace le
