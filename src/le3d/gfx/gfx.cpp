#include <array>
#include <cstddef>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include "le3d/core/assert.hpp"
#include "le3d/context/context.hpp"
#include "le3d/context/contextImpl.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/core/log.hpp"

namespace le::gfx
{
Texture gl::genTex(std::string name, std::string type, std::vector<u8> bytes)
{
	Texture ret;
	if (context::exists())
	{
		s32 w, h, ch;
		stbi_set_flip_vertically_on_load(1);
		auto pData = stbi_load_from_memory(bytes.data(), (s32)bytes.size(), &w, &h, &ch, 0);
		if (pData)
		{
			Lock lock(context::g_glMutex);
			GLObj hTex;
			glChk(glGenTextures(1, &hTex.handle));
			glChk(glActiveTexture(GL_TEXTURE0));
			glChk(glBindTexture(GL_TEXTURE_2D, hTex));
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			glChk(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, ch == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pData));
#if !defined(__arm__)
			glChk(glGenerateMipmap(GL_TEXTURE_2D));
#endif
			glChk(glBindTexture(GL_TEXTURE_2D, 0));
			ret = {std::move(name), std::move(type), std::move(hTex)};
			LOG_I("== [%s] (%s Texture) created [%u]", ret.id.data(), ret.type.data(), ret.glID.handle);
		}
		else
		{
			LOG_E("Failed to load texture!");
		}
		stbi_image_free(pData);
	}
	return ret;
}

void gl::releaseTex(std::vector<Texture*> textures)
{
	if (context::exists())
	{
		std::vector<GLuint> texIDs;
		texIDs.reserve(textures.size());
		Lock lock(context::g_glMutex);
		for (auto pTexture : textures)
		{
			ASSERT(pTexture, "Texture is null!");
			if (pTexture->glID > 0)
			{
				texIDs.push_back(pTexture->glID);
				LOG_I("-- [%s] (Texture) destroyed", pTexture->id.data());
			}
			*pTexture = Texture();
		}
		glChk(glDeleteTextures((GLsizei)texIDs.size(), texIDs.data()));
	}
}

Shader gl::genShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<Shader::MAX_FLAGS> flags)
{
	s32 success;
	if (vertCode.empty())
	{
		LOG_E("[%s] Shader: Failed to compile vertex shader: empty input string!", id.data());
		return {};
	}

#if defined(__arm__)
	static const std::string_view VERSION = "#version 300 es\n";
#else
	static const std::string_view VERSION = "#version 330 core\n";
#endif
	Lock lock(context::g_glMutex);
	u32 vsh = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* files[] = {VERSION.data(), vertCode.data()};
	glShaderSource(vsh, (GLsizei)ARR_SIZE(files), files, nullptr);
	glCompileShader(vsh);
	std::array<char, 512> buf;
	glGetShaderiv(vsh, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vsh, (GLsizei)buf.size(), nullptr, buf.data());
		LOG_E("[%s] (Shader) Failed to compile vertex shader!\n\t%s", id.data(), buf.data());
		return {};
	}

	u32 fsh = glCreateShader(GL_FRAGMENT_SHADER);
	files[1] = fragCode.data();
	glShaderSource(fsh, (GLsizei)ARR_SIZE(files), files, nullptr);
	glCompileShader(fsh);
	glGetShaderiv(fsh, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fsh, (GLsizei)buf.size(), nullptr, buf.data());
		LOG_E("[%s] (Shader) Failed to compile fragment shader!\n\t%s", id.data(), buf.data());
		return {};
	}

	Shader program;
	program.glID = glCreateProgram();
	glAttachShader(program.glID, vsh);
	glAttachShader(program.glID, fsh);
	glLinkProgram(program.glID);
	glGetProgramiv(program.glID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program.glID, (GLsizei)buf.size(), nullptr, buf.data());
		LOG_E("[%s] (Shader) Failed to link shaders!\n\t%s", id.data(), buf.data());
		glDeleteProgram(program.glID);
		return {};
	}

	glDeleteShader(vsh);
	glDeleteShader(fsh);
	LOG_I("== [%s] (Shader) created", id.data());
	program.id = std::move(id);
	program.flags = flags;
	return program;
}

void gl::releaseShader(Shader& shader)
{
	LOG_I("-- [%s] Shader destroyed", shader.id.data());
	glChk(glDeleteProgram(shader.glID));
	shader = Shader();
}

HVerts gl::genVAO(bool bEBO)
{
	HVerts hVerts;
	if (context::exists())
	{
		Lock lock(context::g_glMutex);
		glChk(glGenVertexArrays(1, &hVerts.vao.handle));
		glChk(glGenBuffers(1, &hVerts.vbo.handle));
		if (bEBO)
		{
			glChk(glGenBuffers(1, &hVerts.ebo.handle));
		}
	}
	return hVerts;
}

void gl::releaseVAO(HVerts& hVerts)
{
	if (context::exists() && hVerts.vao > 0)
	{
		Lock lock(context::g_glMutex);
		glChk(glDeleteVertexArrays(1, &hVerts.vao.handle));
		glDeleteBuffers(1, &hVerts.vbo.handle);
		glChk(glDeleteBuffers(1, &hVerts.ebo.handle));
	}
	hVerts = HVerts();
}

void gl::bindBuffers(HVerts& hVerts, std::vector<Vertex> vertices, std::vector<u32> indices)
{
	if (context::exists())
	{
		Lock lock(context::g_glMutex);
		glChk(glBindVertexArray(hVerts.vao));
		glChk(glBindBuffer(GL_ARRAY_BUFFER, hVerts.vbo));
		glChk(glBufferData(GL_ARRAY_BUFFER, (s64)vertices.size() * (s64)sizeof(Vertex), vertices.data(), GL_STATIC_DRAW));
		hVerts.vCount = (u16)vertices.size();
		if (!indices.empty())
		{
			glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hVerts.ebo));
			glChk(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (s64)indices.size() * (s64)sizeof(u32), indices.data(), GL_STATIC_DRAW));
			hVerts.iCount = (u16)indices.size();
		}
		glChk(glBindVertexArray(0));
	}
}

HVerts gl::genVertices(std::vector<Vertex> vertices, std::vector<u32> indices /* = */, const Shader* pShader /* = nullptr */)
{
	HVerts hVerts;
	if (context::exists())
	{
		hVerts = genVAO(!indices.empty());
		bindBuffers(hVerts, vertices, indices);
		Lock lock(context::g_glMutex);
		glChk(glBindVertexArray(hVerts.vao));
		const auto stride = sizeof(Vertex);
		// Colour
		GLint loc = 0;

		// Position
		loc = 0;
		if (pShader)
		{
			loc = glGetAttribLocation(pShader->glID.handle, "aPosition");
		}
		if (loc >= 0)
		{
			glChk(glVertexAttribPointer((u32)loc, 3, GL_FLOAT, GL_FALSE, stride, (void*)(offsetof(Vertex, position))));
			glChk(glEnableVertexAttribArray((u32)loc));
		}

		// Normal
		loc = 1;
		if (pShader)
		{
			loc = glGetAttribLocation(pShader->glID.handle, "aNormal");
		}
		if (loc >= 0)
		{
			glChk(glVertexAttribPointer((u32)loc, 3, GL_FLOAT, GL_FALSE, stride, (void*)(offsetof(Vertex, normal))));
			glChk(glEnableVertexAttribArray((u32)loc));
		}

		// Tex coord
		loc = 2;
		if (pShader)
		{
			loc = glGetAttribLocation(pShader->glID.handle, "aTexCoord");
		}
		if (loc >= 0)
		{
			glChk(glVertexAttribPointer((u32)loc, 2, GL_FLOAT, GL_FALSE, stride, (void*)(offsetof(Vertex, texCoords))));
			glChk(glEnableVertexAttribArray((u32)loc));
		}
	}
	return hVerts;
}

void gl::draw(const HVerts& hVerts, const glm::mat4& m, const glm::mat4& nm, const RenderState& rs, const Shader& s)
{
	Lock lock(context::g_glMutex);
	shading::setupLights(s, rs.dirLights, rs.pointLights);
	auto temp = glGetUniformLocation(s.glID.handle, "model");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(m));
	temp = glGetUniformLocation(s.glID.handle, "normalModel");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(nm));
	temp = glGetUniformLocation(s.glID.handle, "view");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(rs.view));
	temp = glGetUniformLocation(s.glID.handle, "projection");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(rs.projection));
	glChk(glBindVertexArray(hVerts.vao.handle));
	if (hVerts.ebo.handle > 0)
	{
		glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hVerts.ebo.handle));
		glChk(glDrawElements(GL_TRIANGLES, hVerts.iCount, GL_UNSIGNED_INT, 0));
	}
	else
	{
		glChk(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)hVerts.vCount));
	}
	glBindVertexArray(0);
}

void gl::draw(const HVerts& hVerts)
{
	Lock lock(context::g_glMutex);
	glChk(glBindVertexArray(hVerts.vao.handle));
	if (hVerts.ebo.handle > 0)
	{
		glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hVerts.ebo.handle));
		glChk(glDrawElements(GL_TRIANGLES, hVerts.iCount, GL_UNSIGNED_INT, 0));
	}
	else
	{
		glChk(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)hVerts.vCount));
	}
	glBindVertexArray(0);
}

HVerts newVertices(std::vector<Vertex> vertices, std::vector<u32> indices /* =  */, const Shader* pShader /* = nullptr */)
{
	HVerts ret = gl::genVertices(std::move(vertices), std::move(indices), pShader);
	return ret;
}

HVerts tutorial::newLight(const HVerts& hVBO)
{
	HVerts ret;
	if (context::exists())
	{
		ret.vbo = hVBO.vbo;
		ret.vCount = hVBO.vCount;
		const auto stride = sizeof(Vertex);
		Lock lock(context::g_glMutex);
		glChk(glGenVertexArrays(1, &ret.vao.handle));
		glChk(glBindVertexArray(ret.vao));
		glChk(glBindBuffer(GL_ARRAY_BUFFER, ret.vbo));
		glChk(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(offsetof(Vertex, position))));
		glChk(glEnableVertexAttribArray(0));
	}
	return ret;
}
} // namespace le
