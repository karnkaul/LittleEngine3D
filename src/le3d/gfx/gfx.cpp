#include <array>
#include <cstddef>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/context/context.hpp"
#include "le3d/context/contextImpl.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/core/log.hpp"

namespace le
{
namespace gfx
{
GLObj g_blankTexID = GLObj(1);
} // namespace gfx

namespace
{
s32 g_maxTexIdx = 0;

void setBlankTex(const HShader& shader, s32 txID, bool bMagenta, bool& bResetTint)
{
	if (bMagenta)
	{
		gfx::shading::setV4(shader, env::g_config.uniforms.tint, Colour::Magenta);
		bResetTint = true;
	}
	glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
	glChk(glBindTexture(GL_TEXTURE_2D, gfx::g_blankTexID.handle));
}

void setTextures(const HShader& shader, const std::vector<HTexture>& textures, bool& bResetTint)
{
	s32 txID = 0;
	s32 diffuse = 0;
	s32 specular = 0;
	glChk(glBindTexture(GL_TEXTURE_2D, 0));
	const auto& u = env::g_config.uniforms;
	if (!shader.flags.isSet((s32)gfx::shading::Flag::Untextured))
	{
		if (textures.empty())
		{
			setBlankTex(shader, 0, true, bResetTint);
			++txID;
		}
	}
	const size_t idLen = u.material.size() + 1 + std::max({u.diffuseTexPrefix.size() + 2, u.specularTexPrefix.size() + 2});
	for (const auto& texture : textures)
	{
		std::string id;
		id.reserve(idLen);
		id += u.material;
		id += ".";
		std::string number;
		number.reserve(2);
		bool bContinue = false;
		if (txID > g_maxTexIdx)
		{
			g_maxTexIdx = txID;
		}
		switch (texture.type)
		{
		case TexType::Diffuse:
		{
			id += u.diffuseTexPrefix;
			number = std::to_string(diffuse++);
			break;
		}
		case TexType::Specular:
		{
			id += u.specularTexPrefix;

			number = std::to_string(specular++);
			break;
		}
		default:
		{
			if (txID == 0)
			{
				setBlankTex(shader, txID, true, bResetTint);
			}
			bContinue = true;
			break;
		}
		}
		if (bContinue)
		{
			continue;
		}
		id += number;
		ASSERT(id.length() <= idLen, "Incorrect uniform id reserve size!");
		if (texture.glID.handle > 0)
		{
			glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
			glBindTexture(GL_TEXTURE_2D, texture.glID.handle);
			gfx::shading::setS32(shader, id, txID++);
		}
		else
		{
			setBlankTex(shader, txID, true, bResetTint);
		}
	}
	for (; txID <= g_maxTexIdx; ++txID)
	{
		glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
		glChk(glBindTexture(GL_TEXTURE_2D, 0));
	}
}
} // namespace

HTexture gfx::gl::genTexture(std::string name, TexType type, std::vector<u8> bytes, bool bClampToEdge)
{
	HTexture ret;
	if (context::exists())
	{
		s32 w, h, ch;
		stbi_set_flip_vertically_on_load(1);
		auto pData = stbi_load_from_memory(bytes.data(), (s32)bytes.size(), &w, &h, &ch, 0);
		if (pData)
		{
			bool bAlpha = ch > 3;
			Lock lock(context::g_glMutex);
			GLObj hTex;
			glChk(glGenTextures(1, &hTex.handle));
			glChk(glActiveTexture(GL_TEXTURE0));
			glChk(glBindTexture(GL_TEXTURE_2D, hTex));
			if (bClampToEdge)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else
			{
				glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
				glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
			}
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			glChk(glTexImage2D(GL_TEXTURE_2D, 0, bAlpha ? GL_RGBA : GL_RGB, w, h, 0, bAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pData));
#if !defined(__arm__)
			glChk(glGenerateMipmap(GL_TEXTURE_2D));
#endif
			glChk(glBindTexture(GL_TEXTURE_2D, 0));
			ret = {std::move(name), glm::ivec2(w, h), (u32)bytes.size(), type, std::move(hTex)};
			std::string typeStr = ret.type == TexType::Diffuse ? "Diffuse" : "Specular";
			auto size = utils::friendlySize((u64)bytes.size());
			LOG_I("== [%s] [%.1f%s] (%s) Texture created", ret.id.data(), size.first, size.second.data(), typeStr.data());
		}
		else
		{
			LOG_E("Failed to load texture!");
		}
		stbi_image_free(pData);
	}
	return ret;
}

void gfx::gl::releaseTexture(const std::vector<HTexture*>& textures)
{
	if (context::exists())
	{
		std::vector<GLuint> texIDs;
		texIDs.reserve(textures.size());
		Lock lock(context::g_glMutex);
#if defined(DEBUGGING)
		u32 bytes = 0;
#endif
		for (auto pTexture : textures)
		{
			ASSERT(pTexture, "Texture is null!");
			if (pTexture->glID > 0)
			{
				texIDs.push_back(pTexture->glID);
#if defined(DEBUGGING)
				bytes += pTexture->bytes;
#endif
				auto size = utils::friendlySize(pTexture->bytes);
				LOG_I("-- [%s] [%.1f%s] Texture destroyed", pTexture->id.data(), size.first, size.second.data());
			}
			*pTexture = HTexture();
		}
		glChk(glDeleteTextures((GLsizei)texIDs.size(), texIDs.data()));
#if defined(DEBUGGING)
		if (textures.size() > 1)
		{
			auto size = utils::friendlySize(bytes);
			LOG_D("[%.1f%s] Texture VRAM released", size.first, size.second.data());
		}
#endif
	}
}

HCubemap gfx::gl::genCubemap(std::string name, std::array<std::vector<u8>, 6> rlupfb)
{
	HCubemap ret;
	if (context::exists())
	{
		Lock lock(context::g_glMutex);
		glChk(glGenTextures(1, &ret.glID.handle));
		glChk(glBindTexture(GL_TEXTURE_CUBE_MAP, ret.glID.handle));
		s32 w, h, ch;
		u32 idx = 0;
		stbi_set_flip_vertically_on_load(0);
		for (const auto& side : rlupfb)
		{
			auto pData = stbi_load_from_memory(side.data(), (s32)side.size(), &w, &h, &ch, 0);
			if (pData)
			{
				bool bAlpha = ch > 3;
				s32 channels = bAlpha ? GL_RGBA : GL_RGB;
				glChk(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, 0, channels, w, h, 0, (u32)channels, GL_UNSIGNED_BYTE, pData));
				ret.bytes += (u32)side.size();
			}
			else
			{
				LOG_E("Failed to load cubemap texture #%d!", idx);
			}
			++idx;
			stbi_image_free(pData);
		}
		glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		ret.id = std::move(name);
		auto size = utils::friendlySize(ret.bytes);
		LOG_I("== [%s] [%.1f%s] Cubemap created", ret.id.data(), size.first, size.second.data());
	}
	return ret;
}

void gfx::gl::releaseCubemap(HCubemap& cube)
{
	if (context::exists())
	{
		Lock lock(context::g_glMutex);
		GLuint texID[] = {cube.glID.handle};
		glChk(glDeleteTextures(1, texID));
		auto size = utils::friendlySize(cube.bytes);
		LOG_I("-- [%s] [%.1f%s] Cubemap destroyed", cube.id.data(), size.first, size.second.data());
	}
	cube = HCubemap();
}

HShader gfx::gl::genShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags)
{
	if (!context::exists())
	{
		return {};
	}

	s32 success;
	if (vertCode.empty())
	{
		LOG_E("[%s] Shader: Failed to compile vertex shader: empty input string!", id.data());
		return {};
	}
	if (fragCode.empty())
	{
		LOG_E("[%s] Shader: Failed to compile fragment shader: empty input string!", id.data());
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

	HShader program;
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

void gfx::gl::releaseShader(HShader& shader)
{
	LOG_I("-- [%s] Shader destroyed", shader.id.data());
	glChk(glDeleteProgram(shader.glID));
	shader = HShader();
}

HVerts gfx::gl::genVertices(Vertices vertices, Draw drawType, const HShader* pShader)
{
	HVerts hVerts;
	if (context::exists())
	{
		ASSERT(vertices.points.size() % 3 == 0, "Point/normal count mismatch!");
		ASSERT(vertices.normals.empty() || vertices.normals.size() == vertices.points.size(), "Point/normal count mismatch!");
		ASSERT(vertices.texCoords.empty() || 3 * vertices.texCoords.size() == 2 * vertices.points.size(), "Point/UV count mismatch!");
		GLenum type = drawType == Draw::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		Lock lock(context::g_glMutex);
		glChk(glGenVertexArrays(1, &hVerts.vao.handle));
		glChk(glGenBuffers(1, &hVerts.vbo.handle));
		if (!vertices.indices.empty())
		{
			glChk(glGenBuffers(1, &hVerts.ebo.handle));
		}
		glChk(glBindVertexArray(hVerts.vao));
		glChk(glBindBuffer(GL_ARRAY_BUFFER, hVerts.vbo));
		glChk(glBufferData(GL_ARRAY_BUFFER, (s64)vertices.bytes(), nullptr, type));
		size_t sf = (size_t)sizeof(f32);
		auto& p = vertices.points;
		auto& n = vertices.normals;
		auto& t = vertices.texCoords;
		glChk(glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(sf * p.size()), p.data()));
		glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * p.size()), (GLsizeiptr)(sf * n.size()), n.data()));
		glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * p.size() + sf * n.size()), (GLsizeiptr)(sf * t.size()), t.data()));
		hVerts.vCount = (u16)vertices.points.size() / 3;
		if (!vertices.indices.empty())
		{
			glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hVerts.ebo));
			glChk(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (s64)vertices.indices.size() * (s64)sizeof(u32), vertices.indices.data(), type));
			hVerts.iCount = (u16)vertices.indices.size();
		}
		hVerts.bytes = (u32)((vertices.points.size() + vertices.normals.size() + vertices.texCoords.size()) * sizeof(f32))
					   + (u32)(vertices.indices.size() * sizeof(u32));
		// Position
		GLint loc = 0;
		if (pShader)
		{
			loc = glGetAttribLocation(pShader->glID.handle, "aPos");
		}
		if (loc >= 0)
		{
			glChk(glVertexAttribPointer((u32)loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)(3 * sf), 0));
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
			glChk(glVertexAttribPointer((u32)loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)(3 * sf), (void*)(sf * p.size())));
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
			glChk(glVertexAttribPointer((u32)loc, 2, GL_FLOAT, GL_FALSE, (GLsizei)(2 * sf), (void*)(sf * p.size() + sf * n.size())));
			glChk(glEnableVertexAttribArray((u32)loc));
		}

		glChk(glBindVertexArray(0));
	}
	return hVerts;
}

void gfx::gl::releaseVerts(HVerts& hVerts)
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

HUBO gfx::gl::genUBO(s64 size, u32 bindingPoint, Draw type)
{
	HUBO ret;
	if (context::exists())
	{
		Lock lock(context::g_glMutex);
		glChk(glGenBuffers(1, &ret.ubo.handle));
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, ret.ubo.handle));
		GLenum drawType = type == Draw::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glChk(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, drawType));
		glChk(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ret.ubo.handle));
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		ret.bindingPoint = bindingPoint;
		ret.bytes = (u32)size;
	}
	return ret;
}

void gfx::gl::releaseUBO(HUBO& ubo)
{
	if (context::exists() && ubo.ubo > 0)
	{
		Lock lock(context::g_glMutex);
		glChk(glDeleteBuffers(1, &ubo.ubo.handle));
	}
	ubo = HUBO();
}

void gfx::gl::draw(const HVerts& hVerts)
{
	if (context::exists())
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
}

HMesh gfx::newMesh(std::string name, Vertices vertices, Draw type, const HShader* pShader /* = nullptr */)
{
	HMesh mesh;
	if (le::context::exists())
	{
		mesh.name = std::move(name);
		mesh.hVerts = gl::genVertices(std::move(vertices), type, pShader);
		auto size = utils::friendlySize(mesh.hVerts.bytes);
		LOGIF_I(!mesh.name.empty(), "== [%s] [%.1f%s] Mesh set up", mesh.name.data(), size.first, size.second.data());
	}
	return mesh;
}

void gfx::releaseMeshes(const std::vector<HMesh*>& meshes)
{
	for (auto pMesh : meshes)
	{
		if (pMesh->hVerts.vao > 0 && context::exists())
		{
			
			auto size = utils::friendlySize(pMesh->hVerts.bytes);
			LOG_I("-- [%s] [%.1f%s] Mesh destroyed", pMesh->name.data(), size.first, size.second.data());
			gl::releaseVerts(pMesh->hVerts);
		}
		*pMesh = HMesh();
	}
}

void gfx::drawMesh(const HMesh& mesh, const HShader& shader)
{
	if (le::context::exists() && mesh.hVerts.vao.handle > 0)
	{
		bool bResetTint = false;
		const auto& u = env::g_config.uniforms;
		ASSERT(shader.glID.handle > 0, "shader is null!");
		{
			Lock lock(context::g_glMutex);
			gfx::shading::use(shader);
			gfx::shading::setF32(shader, u.shininess, mesh.shininess);
#if defined(DEBUGGING)
			if (mesh.drawFlags.isSet((s32)DrawFlag::Blank) || mesh.drawFlags.isSet((s32)DrawFlag::BlankMagenta))
			{
				setBlankTex(shader, 0, mesh.drawFlags.isSet((s32)DrawFlag::BlankMagenta), bResetTint);
			}
			else
#endif
			{
				setTextures(shader, mesh.textures, bResetTint);
			}
		}
		gl::draw(mesh.hVerts);
		if (bResetTint)
		{
			gfx::shading::setV4(shader, env::g_config.uniforms.tint, Colour::White);
		}
#if defined(DEBUGGING)
		mesh.drawFlags.flags.reset();
#endif
	}
}

void gfx::drawMeshes(const HMesh& mesh, const std::vector<HTexture>& textures, const std::vector<ModelMats>& mats, const HShader& shader)
{
	if (context::exists())
	{
		bool bResetTint = false;
		const auto& u = env::g_config.uniforms;
		ASSERT(shader.glID.handle > 0, "shader is null!");
		{
			Lock lock(context::g_glMutex);
			gfx::shading::use(shader);
			gfx::shading::setF32(shader, u.shininess, mesh.shininess);
#if defined(DEBUGGING)
			if (mesh.drawFlags.isSet((s32)DrawFlag::Blank) || mesh.drawFlags.isSet((s32)DrawFlag::BlankMagenta))
			{
				setBlankTex(shader, 0, mesh.drawFlags.isSet((s32)DrawFlag::BlankMagenta), bResetTint);
			}
			else
#endif
			{
				setTextures(shader, textures, bResetTint);
			}
		}
		for (const auto& mat : mats)
		{
			gfx::shading::setModelMats(shader, mat);
			gl::draw(mesh.hVerts);
		}
		if (bResetTint)
		{
			gfx::shading::setV4(shader, env::g_config.uniforms.tint, Colour::White);
		}
#if defined(DEBUGGING)
		mesh.drawFlags.flags.reset();
#endif
	}
}

HFont gfx::newFont(std::string name, std::vector<u8> spritesheet, glm::ivec2 cellSize)
{
	HFont ret;
	if (context::exists())
	{
		Vertices vertices;
		f32 cellAR = (f32)cellSize.x / cellSize.y;
		f32 width = cellAR < 1.0f ? 1.0f * cellAR : 1.0f;
		f32 height = cellAR > 1.0f ? 1.0f / cellAR : 1.0f;
		ret.quad = createQuad(width, height, name + "_quad");
		ret.sheet = gl::genTexture(name + "_sheet", TexType::Diffuse, std::move(spritesheet), true);
		ret.name = std::move(name);
		ret.cellSize = cellSize;
		LOG_I("== [%s] Font created", ret.name.data());
	}
	return ret;
}

void gfx::releaseFonts(const std::vector<HFont*>& fonts)
{
	if (context::exists())
	{
		std::vector<HMesh*> meshes;
		std::vector<HTexture*> textures;
		meshes.reserve(fonts.size());
		textures.reserve(fonts.size());
		for (auto pFont : fonts)
		{
			meshes.push_back(&pFont->quad);
			textures.push_back(&pFont->sheet);
		}
		releaseMeshes(meshes);
		gl::releaseTexture(textures);
		for (auto pFont : fonts)
		{
			LOG_I("-- [%s] Font destroyed", pFont->name.data());
			*pFont = HFont();
		}
	}
}

HVerts gfx::tutorial::newLight(const HVerts& hVBO)
{
	HVerts ret;
	if (context::exists())
	{
		ret.vbo = hVBO.vbo;
		ret.vCount = hVBO.vCount;
		Lock lock(context::g_glMutex);
		glChk(glGenVertexArrays(1, &ret.vao.handle));
		glChk(glBindVertexArray(ret.vao));
		glChk(glBindBuffer(GL_ARRAY_BUFFER, ret.vbo));
		glChk(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)(3 * sizeof(float)), 0));
		glChk(glEnableVertexAttribArray(0));
	}
	return ret;
}
} // namespace le
