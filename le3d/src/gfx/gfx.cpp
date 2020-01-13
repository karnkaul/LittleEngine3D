#include <array>
#include <cstddef>
#include <sstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/context/context.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/core/log.hpp"
#include "context/contextImpl.hpp"

namespace le
{
namespace
{
s32 g_maxTexIdx = 0;
} // namespace

HTexture gfx::gl::genTexture(std::string name, u8 const* pData, TexType type, u8 ch, u16 w, u16 h, bool bClampToEdge)
{
	if (context::isAlive())
	{
		bool bAlpha = ch > 3;
		GLObj hTex;
		cxChk();
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

#if defined(__arm__)
		glChk(glTexImage2D(GL_TEXTURE_2D, 0, bAlpha ? GL_RGBA : GL_RGB, w, h, 0, bAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pData));
#else
		glChk(glTexImage2D(GL_TEXTURE_2D, 0, bAlpha ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB, w, h, 0, bAlpha ? GL_RGBA : GL_RGB,
						   GL_UNSIGNED_BYTE, pData));
		glChk(glGenerateMipmap(GL_TEXTURE_2D));
#endif
		glChk(glBindTexture(GL_TEXTURE_2D, 0));
		std::string_view typeStr = type == TexType::Diffuse ? "Diffuse" : "Specular";
		u32 size = u32(w * h * ch);
		auto fsize = utils::friendlySize(size);
		LOG_I("== [%s] [%.1f%s] (%s) [%s] created", name.data(), fsize.first, fsize.second.data(), typeStr.data(),
			  Typename<HTexture>().data());
		return {std::move(name), glm::ivec2(w, h), size, type, std::move(hTex)};
	}
	return {};
}

HTexture gfx::gl::genTexture(std::string name, bytestream bytes, TexType type, bool bClampToEdge)
{
	HTexture ret;
	if (context::isAlive())
	{
		cxChk();
		s32 w, h, ch;
		stbi_set_flip_vertically_on_load(1);
		auto* pData = stbi_load_from_memory(bytes.data(), (s32)bytes.size(), &w, &h, &ch, 0);
		if (pData)
		{
			ret = genTexture(std::move(name), pData, type, (u8)ch, (u16)w, (u16)h, bClampToEdge);
		}
		else
		{
			LOG_E("Failed to load texture!");
		}
		stbi_image_free(pData);
	}
	return ret;
}

void gfx::gl::releaseTexture(HTexture& outTexture)
{
	if (contextImpl::exists())
	{
		cxChk();
		if (outTexture.glID > 0)
		{
			auto size = utils::friendlySize(outTexture.byteCount);
			LOG_I("-- [%s] [%.1f%s] [%s] destroyed", outTexture.id.data(), size.first, size.second.data(), Typename<HTexture>().data());
		}
		GLuint t[] = {outTexture.glID};
		glChk(glDeleteTextures(1, t));
	}
	outTexture = HTexture();
}

void gfx::gl::releaseTextures(std::vector<HTexture>& outTextures)
{
	std::vector<GLuint> texIDs;
	texIDs.reserve(outTextures.size());
#if defined(DEBUG_LOG)
	u32 bytes = 0;
#endif
	for (auto& texture : outTextures)
	{
		if (texture.glID.handle > 0)
		{
			texIDs.push_back(texture.glID.handle);
#if defined(DEBUG_LOG)
			bytes += texture.byteCount;
#endif
			auto size = utils::friendlySize(texture.byteCount);
			LOG_I("-- [%s] [%.1f%s] [%s] destroyed", texture.id.data(), size.first, size.second.data(), Typename<HTexture>().data());
		}
		texture = HTexture();
	}
	if (contextImpl::exists())
	{
		cxChk();
		glChk(glDeleteTextures((GLsizei)texIDs.size(), texIDs.data()));
#if defined(DEBUG_LOG)
		if (outTextures.size() > 1)
		{
			auto size = utils::friendlySize(bytes);
			LOG_D("[%.1f%s] Texture VRAM released", size.first, size.second.data());
		}
#endif
	}
}

HCubemap gfx::gl::genCubemap(std::string name, std::array<bytestream, 6> const& rludfb)
{
	HCubemap ret;
	if (context::isAlive())
	{
		cxChk();
		glChk(glGenTextures(1, &ret.glID.handle));
		glChk(glBindTexture(GL_TEXTURE_CUBE_MAP, ret.glID.handle));
		s32 w, h, ch;
		u32 idx = 0;
		u32 inTotal = 0;
		stbi_set_flip_vertically_on_load(0);
		for (auto const& side : rludfb)
		{
			auto pData = stbi_load_from_memory(side.data(), (s32)side.size(), &w, &h, &ch, 0);
			if (pData)
			{
				bool bAlpha = ch > 3;
				s32 channels = bAlpha ? GL_RGBA : GL_RGB;
				glChk(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, 0, channels, w, h, 0, (u32)channels, GL_UNSIGNED_BYTE, pData));
				ret.byteCount += u32(w * h * ch);
				inTotal += side.size();
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
		ret.size = {w, h};
		auto fsize = utils::friendlySize(ret.byteCount);
		auto fin = utils::friendlySize(inTotal);
		LOG_I("== [%s] [%.1f%s => %.1f%s] [%s] created", ret.id.data(), fin.first, fin.second.data(), fsize.first, fsize.second.data(),
			  Typename<HCubemap>().data());
	}
	return ret;
}

void gfx::gl::releaseCubemap(HCubemap& outCube)
{
	if (contextImpl::exists() && outCube.glID > 0)
	{
		cxChk();
		GLuint texID[] = {outCube.glID.handle};
		glChk(glDeleteTextures(1, texID));
		auto size = utils::friendlySize(outCube.byteCount);
		LOG_I("-- [%s] [%.1f%s] [%s] destroyed", outCube.id.data(), size.first, size.second.data(), Typename<HCubemap>().data());
	}
	outCube = HCubemap();
}

HShader gfx::gl::genShader(std::string id, std::string_view vertCode, std::string_view fragCode)
{
	HShader ret;
	if (context::isAlive())
	{
		cxChk();
		s32 success;
		if (vertCode.empty())
		{
			LOG_E("[%s] (Shader) Failed to compile vertex shader: empty input string!", id.data());
			return ret;
		}
		if (fragCode.empty())
		{
			LOG_E("[%s] (Shader) Failed to compile fragment shader: empty input string!", id.data());
			return ret;
		}
#if defined(__arm__)
		static std::string_view const VERSION = "#version 300 es\n";
#else
		static std::string_view const VERSION = "#version 330 core\n";
#endif
		u32 vsh = glCreateShader(GL_VERTEX_SHADER);
		GLchar const* files[] = {VERSION.data(), vertCode.data()};
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
		ret.glID = glCreateProgram();
		glAttachShader(ret.glID, vsh);
		glAttachShader(ret.glID, fsh);
		glLinkProgram(ret.glID);
		glGetProgramiv(ret.glID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ret.glID, (GLsizei)buf.size(), nullptr, buf.data());
			LOG_E("[%s] (Shader) Failed to link shaders!\n\t%s", id.data(), buf.data());
			glDeleteProgram(ret.glID);
			return {};
		}

		glDeleteShader(vsh);
		glDeleteShader(fsh);
		LOG_I("== [%s] [%s] created", id.data(), Typename<HShader>().data());
		ret.id = std::move(id);
	}
	return ret;
}

void gfx::gl::releaseShader(HShader& outShader)
{
	if (contextImpl::exists() && outShader.glID > 0)
	{
		cxChk();
		LOG_I("-- [%s] [%s] destroyed", outShader.id.data(), Typename<HShader>().data());
		glChk(glDeleteProgram(outShader.glID));
	}
	outShader = HShader();
}

HVerts gfx::gl::genVertices(Vertices const& vertices, Draw drawType, HShader const* pShader)
{
	HVerts hVerts;
	if (context::isAlive())
	{
		cxChk();
		ASSERT(vertices.normals.empty() || vertices.normals.size() == vertices.points.size(), "Point/normal count mismatch!");
		ASSERT(vertices.texCoords.empty() || vertices.texCoords.size() == vertices.points.size(), "Point/UV count mismatch!");
		GLenum const type = drawType == Draw::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glChk(glGenVertexArrays(1, &hVerts.vao.handle));
		glChk(glGenBuffers(1, &hVerts.vbo.handle));
		if (!vertices.indices.empty())
		{
			glChk(glGenBuffers(1, &hVerts.ebo.handle));
		}
		glChk(glBindVertexArray(hVerts.vao));
		glChk(glBindBuffer(GL_ARRAY_BUFFER, hVerts.vbo));
		glChk(glBufferData(GL_ARRAY_BUFFER, (s64)vertices.byteCount(), nullptr, type));
		auto const sf = (size_t)sizeof(f32);
		auto const sv3 = (size_t)sizeof(Vertices::V3);
		auto const sv2 = (size_t)sizeof(Vertices::V2);
		auto const& p = vertices.points;
		auto const& n = vertices.normals;
		auto const& t = vertices.texCoords;
		glChk(glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(sv3 * p.size()), p.data()));
		glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sv3 * p.size()), (GLsizeiptr)(sv3 * n.size()), n.data()));
		glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sv3 * (p.size() + n.size())), (GLsizeiptr)(sv2 * t.size()), t.data()));
		hVerts.vCount = (u16)vertices.vertexCount();
		if (!vertices.indices.empty())
		{
			glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hVerts.ebo));
			glChk(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (s64)vertices.indices.size() * (s64)sizeof(u32), vertices.indices.data(), type));
			hVerts.iCount = (u16)vertices.indices.size();
		}
		hVerts.byteCount = vertices.byteCount();
		// Position
		GLint loc = 0;
		if (pShader)
		{
			loc = glGetAttribLocation(pShader->glID.handle, "aPos");
		}
		if (loc >= 0)
		{
			glChk(glVertexAttribPointer((u32)loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)(sv3), 0));
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
			glChk(glVertexAttribPointer((u32)loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)(sv3), (void*)(sv3 * p.size())));
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
			glChk(glVertexAttribPointer((u32)loc, 2, GL_FLOAT, GL_FALSE, (GLsizei)(2 * sf), (void*)(sv3 * (p.size() + n.size()))));
			glChk(glEnableVertexAttribArray((u32)loc));
		}

		glChk(glBindVertexArray(0));
	}
	return hVerts;
}

void gfx::gl::releaseVerts(HVerts& outhVerts)
{
	if (contextImpl::exists() && outhVerts.vao > 0)
	{
		cxChk();
		glChk(glDeleteVertexArrays(1, &outhVerts.vao.handle));
		glDeleteBuffers(1, &outhVerts.vbo.handle);
		glChk(glDeleteBuffers(1, &outhVerts.ebo.handle));
	}
	outhVerts = HVerts();
}

HUBO gfx::gl::genUBO(s64 size, u32 bindingPoint, Draw type)
{
	HUBO ret;
	if (context::isAlive())
	{
		cxChk();
		glChk(glGenBuffers(1, &ret.ubo.handle));
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, ret.ubo.handle));
		GLenum drawType = type == Draw::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glChk(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, drawType));
		glChk(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ret.ubo.handle));
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		ret.bindingPoint = bindingPoint;
		ret.byteCount = (u32)size;
	}
	return ret;
}

void gfx::gl::releaseUBO(HUBO& outhUBO)
{
	if (contextImpl::exists() && outhUBO.ubo > 0)
	{
		cxChk();
		glChk(glDeleteBuffers(1, &outhUBO.ubo.handle));
	}
	outhUBO = HUBO();
}

void gfx::gl::setMaterial(HShader const& shader, Material const& material)
{
	if (context::isAlive())
	{
		auto const& u = env::g_config.uniforms;
		ASSERT(shader.glID.handle > 0, "shader is null!");
		{
			cxChk();
			shader.use();
			bool const bIsLit = material.flags.isSet((s32)Material::Flag::Lit);
			shader.setBool(u.material.isLit, bIsLit);
			bool const bIsTextured = material.flags.isSet((s32)Material::Flag::Textured);
			shader.setBool(u.material.isTextured, bIsTextured);
			if (bIsLit)
			{
				shader.setF32(u.material.shininess, material.shininess);
				shader.setV3(u.material.albedo.ambient, material.albedo.ambient);
				shader.setV3(u.material.albedo.diffuse, material.albedo.diffuse);
				shader.setV3(u.material.albedo.specular, material.albedo.specular);
			}
			if (bIsTextured)
			{
				shader.setBool(u.material.isOpaque, material.flags.isSet((s32)Material::Flag::Opaque));
			}
		}
	}
}

void gfx::gl::draw(HVerts const& hVerts)
{
	if (context::isAlive())
	{
		cxChk();
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

void gfx::setUBO(HUBO const& hUBO, s64 offset, s64 size, void const* pData)
{
	if (hUBO.ubo.handle > 0 && context::isAlive())
	{
		cxChk();
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, hUBO.ubo.handle));
		glChk(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, pData));
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}
}

HMesh gfx::newMesh(std::string name, Vertices const& vertices, Draw type, Material::Flags flags, HShader const* pShader /* = nullptr */)
{
	HMesh mesh;
	if (context::isAlive())
	{
		cxChk();
		mesh.name = std::move(name);
		mesh.hVerts = gl::genVertices(vertices, type, pShader);
		auto size = utils::friendlySize(mesh.hVerts.byteCount);
		LOGIF_I(!mesh.name.empty(), "== [%s] [%.1f%s] [%s] set up (%u vertices)", mesh.name.data(), size.first, size.second.data(),
				Typename<HMesh>().data(), mesh.hVerts.vCount);
		mesh.material.flags = flags;
	}
	return mesh;
}

void gfx::releaseMesh(HMesh& outhMesh)
{
	if (outhMesh.hVerts.vao.handle > 0 && contextImpl::exists())
	{
		cxChk();
		auto size = utils::friendlySize(outhMesh.hVerts.byteCount);
		LOG_I("-- [%s] [%.1f%s] [%s] destroyed", outhMesh.name.data(), size.first, size.second.data(), Typename<HMesh>().data());
		gl::releaseVerts(outhMesh.hVerts);
	}
	outhMesh = HMesh();
}

bool gfx::setTextures(HShader const& shader, std::vector<HTexture> const& textures, bool bSkipIfEmpty)
{
	bool bResetTint = false;
	if (context::isAlive())
	{
		cxChk();
		if (bSkipIfEmpty && textures.empty())
		{
			shader.setS32(env::g_config.uniforms.material.isTextured, 0);
			return false;
		}
		s32 txID = 0;
		s32 diffuse = 0;
		s32 specular = 0;
		auto const& u = env::g_config.uniforms;
		if (textures.empty())
		{
			bResetTint = true;
			setBlankTex(shader, 0, true);
			++txID;
		}
		bool bHasSpecular = false;
		for (auto const& texture : textures)
		{
			std::stringstream id;
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
				id << u.material.diffuseTexPrefix;
				number = std::to_string(diffuse++);
				break;
			}
			case TexType::Specular:
			{
				bHasSpecular = true;
				id << u.material.specularTexPrefix;
				number = std::to_string(specular++);
				break;
			}
			default:
			{
				if (txID == 0)
				{
					bResetTint = true;
					setBlankTex(shader, txID, true);
				}
				bContinue = true;
				break;
			}
			}
			if (bContinue)
			{
				continue;
			}
			id << number;
			if (texture.glID.handle > 0)
			{
				glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
				glBindTexture(GL_TEXTURE_2D, texture.glID.handle);
				shader.setS32(id.str(), txID++);
			}
			else
			{
				bResetTint = true;
				setBlankTex(shader, txID, true);
			}
		}
		shader.setF32(u.material.hasSpecular, bHasSpecular ? 1.0f : 0.0f);
		for (; txID <= g_maxTexIdx; ++txID)
		{
			glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
			glChk(glBindTexture(GL_TEXTURE_2D, 0));
		}
	}
	return bResetTint;
}

void gfx::setBlankTex(HShader const& shader, s32 txID, bool bMagenta)
{
	if (context::isAlive())
	{
		cxChk();
		if (bMagenta)
		{
			shader.setV4(env::g_config.uniforms.tint, Colour::Magenta);
		}
		glChk(glActiveTexture(GL_TEXTURE0 + (u32)txID));
		glChk(glBindTexture(GL_TEXTURE_2D, gfx::g_blankTexID.handle));
	}
}

void gfx::unsetTextures(s32 lastTexID /* = 0 */)
{
	if (context::isAlive())
	{
		cxChk();
		auto resetTex = [](s32 id) {
			glChk(glActiveTexture(GL_TEXTURE0 + (u32)id));
			glChk(glBindTexture(GL_TEXTURE_2D, 0));
		};
		if (lastTexID < 0)
		{
			lastTexID = g_maxTexIdx;
		}
		for (s32 i = 0; i < g_maxTexIdx && i < lastTexID; ++i)
		{
			resetTex(i);
		}
	}
}

void gfx::drawMesh(HMesh const& mesh, HShader const& shader)
{
	cxChk();
	gl::setMaterial(shader, mesh.material);
	gl::draw(mesh.hVerts);
}

void gfx::drawMeshes(HMesh const& mesh, std::vector<ModelMats> const& mats, HShader const& shader)
{
	cxChk();
	gl::setMaterial(shader, mesh.material);
	for (auto const& mat : mats)
	{
		shader.setModelMats(mat);
		gl::draw(mesh.hVerts);
	}
}

HFont gfx::newFont(std::string name, bytestream spritesheet, glm::ivec2 cellSize)
{
	HFont ret;
	if (context::isAlive())
	{
		cxChk();
		Vertices vertices;
		f32 cellAR = (f32)cellSize.x / cellSize.y;
		f32 width = cellAR < 1.0f ? 1.0f * cellAR : 1.0f;
		f32 height = cellAR > 1.0f ? 1.0f / cellAR : 1.0f;
		Material::Flags flags;
		flags.set(s32(Material::Flag::Textured), true);
		ret.quad = createQuad(width, height, name + "_quad", flags);
		ret.sheet = gl::genTexture(name + "_sheet", std::move(spritesheet), TexType::Diffuse, true);
		ret.name = std::move(name);
		ret.cellSize = cellSize;
		LOG_I("== [%s] [%s] created", ret.name.data(), Typename<HFont>().data());
	}
	return ret;
}

void gfx::releaseFont(HFont& outhFont)
{
	if (contextImpl::exists() && outhFont.quad.hVerts.vao.handle > 0 && outhFont.sheet.glID.handle > 0)
	{
		cxChk();
		releaseMesh(outhFont.quad);
		gl::releaseTexture(outhFont.sheet);
		LOG_I("-- [%s] [%s] destroyed", outhFont.name.data(), Typename<HFont>().data());
	}
	outhFont = HFont();
}

HVerts gfx::tutorial::newLight(HVerts const& hVBO)
{
	HVerts ret;
	if (context::isAlive())
	{
		cxChk();
		ret = hVBO;
		glChk(glGenVertexArrays(1, &ret.vao.handle));
		glChk(glBindVertexArray(ret.vao));
		glChk(glBindBuffer(GL_ARRAY_BUFFER, ret.vbo));
		glChk(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)(3 * sizeof(float)), 0));
		glChk(glEnableVertexAttribArray(0));
	}
	return ret;
}
} // namespace le
