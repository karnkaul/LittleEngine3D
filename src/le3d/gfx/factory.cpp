#include <assert.h>
#include <cstddef>
#include <glad/glad.h>
#include <stb/stb_image.h>
#include "le3d/context/context.hpp"
#include "le3d/gfx/factory.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/log/log.hpp"

namespace le
{
HVerts gfx::genVAO(bool bEBO)
{
	HVerts hVerts;
	glGenVertexArrays(1, &hVerts.vao);
	glGenBuffers(1, &hVerts.vbo);
	if (bEBO)
	{
		glGenBuffers(1, &hVerts.ebo);
	}
	return hVerts;
}

void gfx::releaseVAO(HVerts& verts)
{
	if (context::exists() && verts.vao > 0)
	{
		glDeleteVertexArrays(1, &verts.vao);
		glChk();
		glDeleteBuffers(1, &verts.vbo);
		glChk();
		glDeleteBuffers(1, &verts.ebo);
		glChk();
		verts = HVerts();
	}
}

Texture gfx::genTex(std::string name, std::string type, std::vector<u8> bytes)
{
	s32 w, h, ch;
	Texture ret;
	stbi_set_flip_vertically_on_load(1);
	auto pData = stbi_load_from_memory(bytes.data(), toS32(bytes.size()), &w, &h, &ch, 0);
	// auto pData = stbi_load(path.data(), &w, &h, &ch, 0);
	if (pData)
	{
		GLObj hTex = 0;
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
		ret = {hTex, std::move(name), std::move(type)};
		logI("== [%s] %s Texture created [%u]", ret.name.data(), ret.type.data(), ret.id);
	}
	else
	{
		logE("Failed to load texture!");
	}
	stbi_image_free(pData);
	return ret;
}

void gfx::releaseTex(Texture& out_tex)
{
	const GLuint glTex[] = {out_tex.id};
	glDeleteTextures(1, glTex);
	logI("-- [%s] Texture destroyed", out_tex.name.data());
	out_tex = Texture();
}
} // namespace le
