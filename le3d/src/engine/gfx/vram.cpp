#include <stb/stb_image.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/env/env.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfxThread.hpp"
#include "le3d/engine/gfx/le3dgl.hpp"
#include "le3d/engine/gfx/primitives.hpp"
#include "le3d/engine/gfx/vram.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "engine/contextImpl.hpp"

namespace le
{
HVBO gfx::genVec4VBO(descriptors::VBO const& desc, std::vector<GLObj> const& hVAOs)
{
	HVBO vbo;
	if (!context::isAlive())
	{
		return vbo;
	}
	auto f = gfx::enqueue([desc_ = desc, hVAOs_ = hVAOs, &vbo]() {
		static GLint const vaSize = 4;
		GLboolean const glNorm = desc_.bNormalised ? GL_TRUE : GL_FALSE;
		GLsizei const stride = GLsizei(desc_.vec4sPerAttrib * sizeof(glm::vec4));
		vbo.size = desc_.attribCount * (u32)stride;
		vbo.type = desc_.type == DrawType::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glChk(glGenBuffers(1, &vbo.glID.handle));
		glChk(glBindBuffer(GL_ARRAY_BUFFER, vbo.glID));
		glChk(glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(vbo.size), nullptr, GLenum(vbo.type)));
		s32 vaoCount = 0;
		for (auto const& vao : hVAOs_)
		{
			if (vao == 0)
			{
				continue;
			}
			glChk(glBindVertexArray(vao));
			for (u32 idx = 0; idx < desc_.vec4sPerAttrib; ++idx)
			{
				auto offset = idx * sizeof(glm::vec4);
				u32 loc = u32(desc_.attribLocation) + idx;
				glChk(glVertexAttribPointer(loc, vaSize, GL_FLOAT, glNorm, (GLsizei)(stride), (void*)offset));
				glChk(glEnableVertexAttribArray(loc));
				if (desc_.attribDivisor > 0)
				{
					glChk(glVertexAttribDivisor(loc, desc_.attribDivisor));
				}
				if (vaoCount == 0)
				{
					vbo.vaIDs.push_back(loc);
				}
			}
			++vaoCount;
		}
		glChk(glBindVertexArray(0));
	});
	gfx::wait(f);
	return vbo;
}

void gfx::setVBO(HVBO const& hVBO, void const* pData)
{
	if (context::isAlive() && hVBO.glID > 0)
	{
		std::vector<u8> bytes((size_t)hVBO.size, 0);
		memcpy(bytes.data(), pData, (size_t)hVBO.size);
		gfx::enqueue([hVBO = hVBO, bytes = std::move(bytes)]() {
			glChk(glBufferData(GL_ARRAY_BUFFER, (s64)hVBO.size, bytes.data(), (GLenum)hVBO.type));
		});
	}
	return;
}

void gfx::releaseVBO(HVBO& outVBO, std::vector<GLObj> const& hVAOs)
{
	if (contextImpl::exists())
	{
		gfx::enqueue([vbo = outVBO, hVAOs = hVAOs]() {
			for (auto const& vao : hVAOs)
			{
				if (vao == 0)
				{
					continue;
				}
				glChk(glBindVertexArray(vao));
				for (auto const& vaID : vbo.vaIDs)
				{
					glChk(glDisableVertexAttribArray(vaID));
				}
			}
			glChk(glDeleteBuffers(1, &vbo.glID.handle));
			glChk(glBindVertexArray(0));
		});
	}
	outVBO = HVBO();
	return;
}

HVerts gfx::genVerts(Vertices const& vertices, DrawType drawType, HShader const* pShader)
{
	HVerts hVerts;
	if (context::isAlive())
	{
		ASSERT(vertices.normals.empty() || vertices.normals.size() == vertices.points.size(), "Point/normal count mismatch!");
		ASSERT(vertices.texCoords.empty() || vertices.texCoords.size() == vertices.points.size(), "Point/UV count mismatch!");
		hVerts.hVBO.type = drawType == DrawType::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		hVerts.hVBO.size = vertices.byteCount();
		auto f = gfx::enqueue([&]() {
			glChk(glGenVertexArrays(1, &hVerts.hVAO.handle));
			glChk(glGenBuffers(1, &hVerts.hVBO.glID.handle));
			if (!vertices.indices.empty())
			{
				glChk(glGenBuffers(1, &hVerts.hEBO.handle));
			}
			glChk(glBindVertexArray(hVerts.hVAO));
			glChk(glBindBuffer(GL_ARRAY_BUFFER, hVerts.hVBO.glID));
			glChk(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertices.byteCount(), nullptr, (GLenum)hVerts.hVBO.type));
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
				GLsizeiptr size = GLsizeiptr(vertices.indices.size() * sizeof(u32));
				glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hVerts.hEBO));
				glChk(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, vertices.indices.data(), (GLenum)hVerts.hVBO.type));
				hVerts.iCount = (u16)vertices.indices.size();
			}
			hVerts.byteCount = vertices.byteCount();
			// Position
			GLint loc = 0;
			if (pShader)
			{
				loc = glGetAttribLocation(pShader->glID, "aPos");
			}
			if (loc >= 0)
			{
				glChk(glVertexAttribPointer((GLuint)loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)(sv3), 0));
				glChk(glEnableVertexAttribArray((GLuint)loc));
				hVerts.hVBO.vaIDs.push_back((u32)loc);
			}

			// Normal
			loc = 1;
			if (pShader)
			{
				loc = glGetAttribLocation(pShader->glID, "aNormal");
			}
			if (loc >= 0)
			{
				glChk(glVertexAttribPointer((GLuint)loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)(sv3), (void*)(sv3 * p.size())));
				glChk(glEnableVertexAttribArray((u32)loc));
				hVerts.hVBO.vaIDs.push_back((u32)loc);
			}

			// Tex coord
			loc = 2;
			if (pShader)
			{
				loc = glGetAttribLocation(pShader->glID, "aTexCoord");
			}
			if (loc >= 0)
			{
				glChk(glVertexAttribPointer((GLuint)loc, 2, GL_FLOAT, GL_FALSE, (GLsizei)(2 * sf), (void*)(sv3 * (p.size() + n.size()))));
				glChk(glEnableVertexAttribArray((u32)loc));
				hVerts.hVBO.vaIDs.push_back((u32)loc);
			}

			glChk(glBindVertexArray(0));
		});
		gfx::wait(f);
	}
	return hVerts;
}

void gfx::releaseVerts(HVerts& outhVerts)
{
	if (contextImpl::exists() && outhVerts.hVAO > 0)
	{
		gfx::enqueue([hVerts = outhVerts]() {
			glChk(glDeleteVertexArrays(1, &hVerts.hVAO.handle));
			glChk(glDeleteBuffers(1, &hVerts.hVBO.glID.handle));
			glChk(glDeleteBuffers(1, &hVerts.hEBO.handle));
		});
	}
	outhVerts = HVerts();
	return;
}

HSampler gfx::genSampler(std::string id, descriptors::Sampler const& desc)
{
	HSampler hSampler;
	if (context::isAlive())
	{
		GLint glWrap;
		switch (desc.wrap)
		{
		case TexWrap::ClampEdge:
			glWrap = GL_CLAMP_TO_EDGE;
			break;

		case TexWrap::ClampBorder:
			glWrap = GL_CLAMP_TO_BORDER;
			break;

		default:
		case TexWrap::Repeat:
			glWrap = GL_REPEAT;
			break;
		}
		auto getGLFilter = [](TexFilter filter) -> GLint {
			switch (filter)
			{
			default:
			case TexFilter::Linear:
				return GL_LINEAR;
			case TexFilter::Nearest:
				return GL_NEAREST;
			case TexFilter::LinearMpLinear:
				return GL_LINEAR_MIPMAP_LINEAR;
			case TexFilter::LinearMpNearest:
				return GL_LINEAR_MIPMAP_NEAREST;
			case TexFilter::NearestMpLinear:
				return GL_NEAREST_MIPMAP_LINEAR;
			case TexFilter::NearestMpNearest:
				return GL_LINEAR_MIPMAP_NEAREST;
			}
		};
		GLint glMinFilter = getGLFilter(desc.minFilter);
		GLint glMagFilter = getGLFilter(desc.magFilter);
		auto f = gfx::enqueue([&]() {
			glChk(glGenSamplers(1, &hSampler.glID.handle));
			glChk(glSamplerParameteri(hSampler.glID, GL_TEXTURE_WRAP_S, glWrap));
			glChk(glSamplerParameteri(hSampler.glID, GL_TEXTURE_WRAP_T, glWrap));
			glChk(glSamplerParameteri(hSampler.glID, GL_TEXTURE_MIN_FILTER, glMinFilter));
			glChk(glSamplerParameteri(hSampler.glID, GL_TEXTURE_MAG_FILTER, glMagFilter));
#if defined(LE3D_USE_GLAD) && defined(GL_ARB_texture_filter_anisotropic)
			if (GLAD_GL_ARB_texture_filter_anisotropic == 1)
			{
				glChk(glSamplerParameteri(hSampler.glID, GL_TEXTURE_MAX_ANISOTROPY, desc.anisotropy));
			}
#endif
		});
		gfx::wait(f);
		hSampler.id = std::move(id);
		LOG_I("== [%s] [%s] created", hSampler.id.data(), typeName<HSampler>().data());
	}
	return hSampler;
}

void gfx::releaseSampler(HSampler& outhSampler)
{
	if (contextImpl::exists() && outhSampler.glID > 0)
	{
		gfx::enqueue([hSampler = outhSampler]() { glDeleteSamplers(1, &hSampler.glID.handle); });
		LOG_I("-- [%s] [%s] destroyed", outhSampler.id.data(), typeName<HSampler>().data());
	}
	outhSampler = HSampler();
	return;
}

HTexture gfx::genTexture(std::string id, u8 const* pData, TexType type, HSampler* pSampler, u8 ch, u16 w, u16 h)
{
	HTexture ret;
	if (context::isAlive())
	{
		bool bAlpha = ch > 3;
		auto f = gfx::enqueue([&]() {
			glChk(glGenTextures(1, &ret.glID.handle));
			glChk(glActiveTexture(GL_TEXTURE0));
			glChk(glBindTexture(GL_TEXTURE_2D, ret.glID));
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GLint const extFormat = bAlpha ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB;
			GLenum const intFormat = bAlpha ? GL_RGBA : GL_RGB;
			glChk(glTexImage2D(GL_TEXTURE_2D, 0, extFormat, w, h, 0, intFormat, GL_UNSIGNED_BYTE, pData));
			glChk(glGenerateMipmap(GL_TEXTURE_2D));
			glChk(glBindTexture(GL_TEXTURE_2D, 0));
		});
		gfx::wait(f);
		std::string_view typeStr = type == TexType::Diffuse ? "Diffuse" : "Specular";
		u32 size = u32(w * h * ch);
		auto fsize = utils::friendlySize(size);
		LOG_I("== [%s] [%.1f%s] (%s) [%s] created", id.data(), fsize.first, fsize.second.data(), typeStr.data(),
			  typeName<HTexture>().data());
		ret.id = std::move(id);
		ret.byteCount = size;
		ret.type = type;
		ret.size = {w, h};
		if (pSampler)
		{
			ret.hSampler = *pSampler;
		}
	}
	return ret;
}

HTexture gfx::genTexture(std::string id, bytearray bytes, TexType type, HSampler* pSampler)
{
	HTexture ret;
	if (context::isAlive())
	{
		s32 w, h, ch;
		stbi_set_flip_vertically_on_load(1);
		auto* pData = stbi_load_from_memory(reinterpret_cast<u8 const*>(bytes.data()), (s32)bytes.size(), &w, &h, &ch, 0);
		if (pData)
		{
			ret = genTexture(std::move(id), pData, type, pSampler, (u8)ch, (u16)w, (u16)h);
		}
		else
		{
			LOG_E("Failed to load texture!");
		}
		stbi_image_free(pData);
	}
	return ret;
}

void gfx::releaseTexture(HTexture& outhTexture)
{
	if (contextImpl::exists())
	{
		if (outhTexture.glID > 0)
		{
			auto size = utils::friendlySize(outhTexture.byteCount);
			LOG_I("-- [%s] [%.1f%s] [%s] destroyed", outhTexture.id.data(), size.first, size.second.data(), typeName<HTexture>().data());
		}
		gfx::enqueue([hTexture = outhTexture]() {
			GLuint t[] = {hTexture.glID};
			glChk(glDeleteTextures(1, t));
		});
	}
	outhTexture = HTexture();
	return;
}

void gfx::releaseTextures(std::vector<HTexture>& outhTextures)
{
	std::vector<GLuint> texIDs;
	texIDs.reserve(outhTextures.size());
#if defined(DEBUG_LOG)
	u32 bytes = 0;
#endif
	for (auto& texture : outhTextures)
	{
		if (texture.glID > 0)
		{
			texIDs.push_back(texture.glID);
#if defined(DEBUG_LOG)
			bytes += texture.byteCount;
#endif
			auto size = utils::friendlySize(texture.byteCount);
			LOG_I("-- [%s] [%.1f%s] [%s] destroyed", texture.id.data(), size.first, size.second.data(), typeName<HTexture>().data());
		}
		texture = HTexture();
	}
	if (contextImpl::exists())
	{
		gfx::enqueue([texIDs_ = std::move(texIDs)]() { glChk(glDeleteTextures((GLsizei)texIDs_.size(), texIDs_.data())); });
#if defined(DEBUG_LOG)
		if (outhTextures.size() > 1)
		{
			auto size = utils::friendlySize(bytes);
			LOG_D("[%.1f%s] Texture VRAM released", size.first, size.second.data());
		}
#endif
	}
	return;
}

HCubemap gfx::genCubemap(std::string id, std::array<bytearray, 6> const& rludfb)
{
	HCubemap ret;
	if (context::isAlive())
	{
		auto f = gfx::enqueue([&]() {
			glChk(glGenTextures(1, &ret.glID.handle));
			glChk(glBindTexture(GL_TEXTURE_CUBE_MAP, ret.glID));
		});
		gfx::wait(f);
		s32 w, h, ch;
		u32 idx = 0;
		u32 inTotal = 0;
		stbi_set_flip_vertically_on_load(0);
		for (auto const& side : rludfb)
		{
			auto pData = stbi_load_from_memory(reinterpret_cast<u8 const*>(side.data()), (s32)side.size(), &w, &h, &ch, 0);
			if (pData)
			{
				bool bAlpha = ch > 3;
				s32 channels = bAlpha ? GL_RGBA : GL_RGB;
				f = gfx::enqueue([&]() {
					glChk(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, 0, channels, w, h, 0, (u32)channels, GL_UNSIGNED_BYTE, pData));
				});
				gfx::wait(f);
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
		f = gfx::enqueue([&]() {
			glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		});
		gfx::wait(f);
		ret.id = std::move(id);
		ret.size = {w, h};
		auto fsize = utils::friendlySize(ret.byteCount);
		auto fin = utils::friendlySize(inTotal);
		LOG_I("== [%s] [%.1f%s => %.1f%s] [%s] created", ret.id.data(), fin.first, fin.second.data(), fsize.first, fsize.second.data(),
			  typeName<HCubemap>().data());
	}
	return ret;
}

void gfx::releaseCubemap(HCubemap& outhCube)
{
	if (contextImpl::exists() && outhCube.glID > 0)
	{
		gfx::enqueue([hCube = outhCube]() {
			GLuint texID[] = {hCube.glID};
			glChk(glDeleteTextures(1, texID));
		});
		auto size = utils::friendlySize(outhCube.byteCount);
		LOG_I("-- [%s] [%.1f%s] [%s] destroyed", outhCube.id.data(), size.first, size.second.data(), typeName<HCubemap>().data());
	}
	outhCube = HCubemap();
	return;
}

HShader gfx::genShader(std::string id, std::string_view vertCode, std::string_view fragCode)
{
	HShader ret;
	if (context::isAlive())
	{
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
		u32 vsh = 0;
		GLchar const* files[] = {env::g_config.shaderPrefix.data(), "\n", vertCode.data()};
		size_t const filesSize = ARR_SIZE(files);
		std::array<char, 512> buf;
		auto f = gfx::enqueue([&]() {
			vsh = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vsh, (GLsizei)filesSize, files, nullptr);
			glCompileShader(vsh);
			glGetShaderiv(vsh, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vsh, (GLsizei)buf.size(), nullptr, buf.data());
				glDeleteShader(vsh);
			}
			if (!success)
			{
				LOG_E("[%s] (Shader) Failed to compile vertex shader!\n\t%s", id.data(), buf.data());
			}
			u32 fsh = 0;
			fsh = glCreateShader(GL_FRAGMENT_SHADER);
			files[filesSize - 1] = fragCode.data();
			glShaderSource(fsh, (GLsizei)filesSize, files, nullptr);
			glCompileShader(fsh);
			glGetShaderiv(fsh, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glDeleteShader(vsh);
				glGetShaderInfoLog(fsh, (GLsizei)buf.size(), nullptr, buf.data());
				glDeleteShader(fsh);
			}
			if (!success)
			{
				LOG_E("[%s] (Shader) Failed to compile fragment shader!\n\t%s", id.data(), buf.data());
			}
			ret.glID = glCreateProgram();
			glAttachShader(ret.glID, vsh);
			glAttachShader(ret.glID, fsh);
			glLinkProgram(ret.glID);
			glGetProgramiv(ret.glID, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(ret.glID, (GLsizei)buf.size(), nullptr, buf.data());
				glDeleteProgram(ret.glID);
			}
			glDeleteShader(vsh);
			glDeleteShader(fsh);
		});
		gfx::wait(f);
		if (!success)
		{
			LOG_E("[%s] (Shader) Failed to link shaders!\n\t%s", id.data(), buf.data());
			return {};
		}
		LOG_I("== [%s] [%s] created", id.data(), typeName<HShader>().data());
		ret.id = std::move(id);
	}
	return ret;
}

void gfx::releaseShader(HShader& outhShader)
{
	if (contextImpl::exists() && outhShader.glID > 0)
	{
		LOG_I("-- [%s] [%s] destroyed", outhShader.id.data(), typeName<HShader>().data());
		gfx::enqueue([hShader = outhShader]() { glChk(glDeleteProgram(hShader.glID)); });
	}
	outhShader = HShader();
	return;
}

HUBO gfx::genUBO(std::string id, s64 size, u32 bindingPoint, DrawType type)
{
	HUBO ret;
	if (context::isAlive())
	{
		auto f = gfx::enqueue([&]() {
			glChk(glGenBuffers(1, &ret.glID.handle));
			glChk(glBindBuffer(GL_UNIFORM_BUFFER, ret.glID));
			GLenum drawType = type == DrawType::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
			glChk(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, drawType));
			glChk(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ret.glID));
			glChk(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		});
		gfx::wait(f);
		ret.id = std::move(id);
		ret.bindingPoint = bindingPoint;
		ret.byteCount = (u32)size;
		auto fsize = utils::friendlySize((u32)size);
		LOG_I("== [%s] [%.1f%s] [%s] (%d) added for future shaders", ret.id.data(), fsize.first, fsize.second.data(),
			  typeName<HUBO>().data(), ret.bindingPoint);
	}
	return ret;
}

void gfx::setUBO(HUBO const& hUBO, s64 offset, s64 size, void const* pData)
{
	if (hUBO.glID > 0 && context::isAlive())
	{
		std::vector<u8> data((size_t)size);
		memcpy(data.data(), pData, (size_t)size);
		gfx::enqueue([hUBO_ = hUBO, offset, size, data_ = std::move(data)]() {
			glChk(glBindBuffer(GL_UNIFORM_BUFFER, hUBO_.glID));
			glChk(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data_.data()));
			glChk(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		});
	}
	return;
}

void gfx::releaseUBO(HUBO& outhUBO)
{
	if (contextImpl::exists() && outhUBO.glID > 0)
	{
		auto size = utils::friendlySize(outhUBO.byteCount);
		LOG_I("-- [%s] [%.1f%s] [%s] (%d) destroyed", outhUBO.id.data(), size.first, size.second.data(), typeName<HUBO>().data(),
			  outhUBO.bindingPoint);
		gfx::enqueue([hUBO = outhUBO]() { glChk(glDeleteBuffers(1, &hUBO.glID.handle)); });
	}
	outhUBO = HUBO();
	return;
}

Mesh gfx::newMesh(std::string id, Vertices const& vertices, DrawType type, Material::Flags const& flags,
				  HShader const* pShader /* = nullptr */)
{
	Mesh mesh;
	if (context::isAlive())
	{
		mesh.m_id = std::move(id);
		mesh.m_hVerts = genVerts(vertices, type, pShader);
		auto size = utils::friendlySize(mesh.m_hVerts.byteCount);
		LOGIF_I(!mesh.m_id.empty(), "== [%s] [%.1f%s] [%s] set up (%u vertices)", mesh.m_id.data(), size.first, size.second.data(),
				typeName<Mesh>().data(), mesh.m_hVerts.vCount);
		mesh.m_material.flags = flags;
	}
	return mesh;
}

void gfx::releaseMesh(Mesh& outMesh)
{
	if (outMesh.m_hVerts.hVAO > 0 && contextImpl::exists())
	{
		auto size = utils::friendlySize(outMesh.m_hVerts.byteCount);
		LOG_I("-- [%s] [%.1f%s] [%s] destroyed", outMesh.m_id.data(), size.first, size.second.data(), typeName<Mesh>().data());
		releaseVerts(outMesh.m_hVerts);
	}
	outMesh = Mesh();
	return;
}

BitmapFont gfx::newFont(std::string id, bytearray spritesheet, glm::ivec2 cellSize, HSampler const& hSampler)
{
	BitmapFont ret;
	if (context::isAlive())
	{
		Vertices vertices;
		f32 cellAR = (f32)cellSize.x / cellSize.y;
		f32 width = cellAR < 1.0f ? 1.0f * cellAR : 1.0f;
		f32 height = cellAR > 1.0f ? 1.0f / cellAR : 1.0f;
		Material::Flags flags;
		flags.set(Material::Flag::Textured, true);
		ret.quad = createQuad(width, height, id + "_quad", flags);
		ret.sheet = genTexture(id + "_sheet", std::move(spritesheet), TexType::Diffuse);
		ret.sheet.hSampler = hSampler;
		ret.id = std::move(id);
		ret.cellSize = cellSize;
		LOG_I("== [%s] [%s] created", ret.id.data(), typeName<BitmapFont>().data());
	}
	return ret;
}

void gfx::releaseFont(BitmapFont& outFont)
{
	if (contextImpl::exists() && outFont.quad.m_hVerts.hVAO > 0 && outFont.sheet.glID > 0)
	{
		releaseMesh(outFont.quad);
		releaseTexture(outFont.sheet);
		LOG_I("-- [%s] [%s] destroyed", outFont.id.data(), typeName<BitmapFont>().data());
	}
	outFont = BitmapFont();
	return;
}
} // namespace le
