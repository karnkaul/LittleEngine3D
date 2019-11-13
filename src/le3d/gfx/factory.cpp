#include <assert.h>
#include <cstddef>
#include <glad/glad.h>
#include <stb/stb_image.h>
#include "le3d/context/context.hpp"
#include "le3d/context/contextImpl.hpp"
#include "le3d/gfx/factory.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/core/log.hpp"

namespace le
{
HVerts gfx::genVAO(bool bEBO)
{
	Lock lock(context::g_glMutex);
	HVerts hVerts;
	glChk(glGenVertexArrays(1, &hVerts.vao));
	glChk(glGenBuffers(1, &hVerts.vbo));
	if (bEBO)
	{
		glChk(glGenBuffers(1, &hVerts.ebo));
	}
	return hVerts;
}

void gfx::releaseVAO(HVerts& verts)
{
	if (context::exists() && verts.vao > 0)
	{
		Lock lock(context::g_glMutex);
		glChk(glDeleteVertexArrays(1, &verts.vao));
		glDeleteBuffers(1, &verts.vbo);
		glChk(glDeleteBuffers(1, &verts.ebo));
		verts = HVerts();
	}
}

Texture gfx::genTex(std::string name, std::string type, std::vector<u8> bytes)
{
	s32 w, h, ch;
	Texture ret;
	stbi_set_flip_vertically_on_load(1);
	auto pData = stbi_load_from_memory(bytes.data(), (s32)bytes.size(), &w, &h, &ch, 0);
	if (pData)
	{
		Lock lock(context::g_glMutex);
		GLObj hTex = 0;
		glChk(glGenTextures(1, &hTex));
		glChk(glActiveTexture(GL_TEXTURE0));
		glChk(glBindTexture(GL_TEXTURE_2D, hTex));
		glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glChk(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, ch == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pData));
		glChk(glGenerateMipmap(GL_TEXTURE_2D));
		glChk(glBindTexture(GL_TEXTURE_2D, 0));
		ret = {hTex, std::move(name), std::move(type)};
		LOG_I("== [%s] %s Texture created [%u]", ret.name.data(), ret.type.data(), ret.id);
	}
	else
	{
		LOG_E("Failed to load texture!");
	}
	stbi_image_free(pData);
	return ret;
}

void gfx::releaseTex(Texture& out_tex)
{
	Lock lock(context::g_glMutex);
	const GLuint glTex[] = {out_tex.id};
	glChk(glDeleteTextures(1, glTex));
	LOG_I("-- [%s] Texture destroyed", out_tex.name.data());
	out_tex = Texture();
}
} // namespace le
