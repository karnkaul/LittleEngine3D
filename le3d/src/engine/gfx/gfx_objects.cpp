#include <algorithm>
#include <array>
#include <unordered_map>
#include <glad/glad.h>
#include <stb/stb_image.h>
#include "le3d/defines.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfx_thread.hpp"
#include "le3d/engine/gfx/gfx_objects.hpp"
#include "le3d/engine/gfx/gfx_store.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/env/env.hpp"
#include "engine/context_impl.hpp"
#include "engine/gfx/le3dgl.hpp"

namespace le::gfx
{
#if defined(LE3D_GFX_DEBUG_LOGS)
#define LOGIF_X_Y(bTest, T, action, glID) LOGIF_D(bTest, "[%s] " action " [%u]", typeName<T>().data(), glID.handle)
#define LOG_SETUP_ENTER(T, id) LOG_D("[%s] Entered ::setup() [%s]", typeName<T>().data(), id.generic_string().data())
#define LOG_SETUP_EXIT(T, id) LOG_D("[%s] Exiting ::setup() [%s]", typeName<T>().data(), id.generic_string().data())
#else
#define LOGIF_X_Y(bTest, T, action, glID)
#define LOG_SETUP_ENTER(T, id)
#define LOG_SETUP_EXIT(T, id)
#endif

namespace
{
std::mutex g_stbiMutex;
using Lock = std::lock_guard<std::mutex>;

template <typename Enum, typename Arr>
Enum strToEnum(Arr arr, std::string_view value)
{
	for (size_t idx = 0; idx < arr.size(); ++idx)
	{
		if (arr.at(idx) == value)
		{
			return (Enum)idx;
		}
	}
	return (Enum)0;
}

using TexTypeArr = std::array<std::string_view, (size_t)TexType::COUNT_>;
TexTypeArr g_texTypes = {"diffuse", "specular"};
TexType texType(std::string_view type)
{
	return strToEnum<TexType, TexTypeArr>(g_texTypes, type);
}

using TexWrapArr = std::array<std::string_view, (size_t)TexWrap::COUNT_>;
TexWrapArr g_texWraps = {"repeat", "clampedge", "clampborder"};
TexWrap texWrap(std::string_view type)
{
	return strToEnum<TexWrap, TexWrapArr>(g_texWraps, type);
}

using TexFilterArr = std::array<std::string_view, (size_t)TexFilter::COUNT_>;
TexFilterArr g_texFilters = {"linear", "nearest", "linearmplinear", "linearmpnearest", "nearestmplinear", "nearestmpnearest"};
TexFilter texFilter(std::string_view type)
{
	return strToEnum<TexFilter, TexFilterArr>(g_texFilters, type);
}

GFXID g_activeShader;
std::array<std::pair<GFXID, GFXID>, 128> g_activeTextureSampler;

void setTexture(s32 unit, GFXID const& samplerID, GFXID const& textureID)
{
	auto const& pair = g_activeTextureSampler.at((size_t)unit);
	if (pair.first != samplerID || pair.second != textureID)
	{
		g_activeTextureSampler.at((size_t)unit) = {samplerID, textureID};
		gfx::enqueue([unit, sID = samplerID, tID = textureID]() {
			glChk(glActiveTexture(GL_TEXTURE0 + (GLuint)unit));
			glChk(glBindSampler((GLuint)unit, sID));
			glChk(glBindTexture(GL_TEXTURE_2D, tID));
		});
	}
	return;
}

glm::vec2 getTextTLOffset(Font::Text::HAlign h, Font::Text::VAlign v)
{
	glm::vec2 textTLoffset = glm::vec2(0.0f);
	switch (h)
	{
	case Font::Text::HAlign::Centre:
	{
		textTLoffset.x = -0.5f;
		break;
	}
	case Font::Text::HAlign::Left:
	default:
		break;

	case Font::Text::HAlign::Right:
	{
		textTLoffset.x = -1.0f;
		break;
	}
	}
	switch (v)
	{
	case Font::Text::VAlign::Middle:
	{
		textTLoffset.y = 0.5f;
		break;
	}
	default:
	case Font::Text::VAlign::Top:
	{
		break;
	}
	case Font::Text::VAlign::Bottom:
	{
		textTLoffset.y = 1.0f;
		break;
	}
	}
	return textTLoffset;
}
} // namespace

u32 Geometry::byteCount() const
{
	return (u32)((points.size() + normals.size()) * sizeof(V3) + texCoords.size() * sizeof(V2));
}

u32 Geometry::vertexCount() const
{
	return (u32)points.size();
}

void Geometry::addPoint(glm::vec3 const& point)
{
	points.push_back({point.x, point.y, point.z});
	return;
}

void Geometry::addNormals(glm::vec3 const& normal, u16 count)
{
	for (u16 i = 0; i < count; ++i)
	{
		normals.push_back({normal.x, normal.y, normal.z});
	}
	return;
}

void Geometry::addTexCoord(glm::vec2 const& texCoord)
{
	texCoords.push_back({texCoord.x, texCoord.y});
	return;
}

void Geometry::reserve(u32 vCount, u32 iCount)
{
	points.reserve(vCount);
	normals.reserve(vCount);
	texCoords.reserve(vCount);
	indices.reserve(iCount);
	return;
}

u32 Geometry::addVertex(glm::vec3 const& point, glm::vec3 const& normal, std::optional<glm::vec2> oTexCoord)
{
	points.push_back({point.x, point.y, point.z});
	normals.push_back({normal.x, normal.y, normal.z});
	if (oTexCoord)
	{
		texCoords.push_back({oTexCoord->x, oTexCoord->y});
	}
	return (u32)points.size() - 1;
}

void Geometry::addIndices(std::vector<u32> const& newIndices)
{
	std::copy(newIndices.begin(), newIndices.end(), std::back_inserter(indices));
	return;
}

bool operator==(Geometry::V3 const& lhs, Geometry::V3 const& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool operator==(Geometry::V2 const& lhs, Geometry::V2 const& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(Geometry::V3 const& lhs, Geometry::V3 const& rhs)
{
	return !(lhs == rhs);
}

bool operator!=(Geometry::V2 const& lhs, Geometry::V2 const& rhs)
{
	return !(lhs == rhs);
}

u32 InstanceBuffer::instanceCount() const
{
	return (u32)models.size();
}

void Material::deserialise(JSONObj const& serialised)
{
	id = serialised.getString("id", "UNNAMED");
	if (serialised.contains("albedo"))
	{
		auto albedoData = serialised.getGData("albedo");
		albedo.shininess = (f32)albedoData.getF64("shininess", albedo.shininess);
		if (albedoData.contains("ambient"))
		{
			auto setColour = [&](std::string const& id, glm::vec3& vec) {
				if (albedoData.contains(id))
				{
					auto colourData = albedoData.getGData(id);
					vec.r = (f32)colourData.getF64("r", vec.r);
					vec.g = (f32)colourData.getF64("g", vec.g);
					vec.b = (f32)colourData.getF64("b", vec.b);
				}
			};
			setColour("ambient", albedo.ambient);
			setColour("diffuse", albedo.diffuse);
			setColour("specular", albedo.specular);
		}
	}
	if (serialised.contains("flags"))
	{
		auto flagList = serialised.getVecString("flags");
		for (auto& flag : flagList)
		{
			utils::strings::toLower(flag);
			if (flag == "lit")
			{
				flags.set(Flag::Lit, true);
			}
			else if (flag == "textured")
			{
				flags.set(Flag::Textured, true);
			}
			else if (flag == "opaque")
			{
				flags.set(Flag::Opaque, true);
			}
			else if (flag == "specular")
			{
				flags.set(Flag::Specular, true);
			}
			else
			{
				LOG_W("[%s] [%s] Unrecognised Material flag: [%s]!", typeName<Material>().data(), id.generic_string().data(), flag.data());
			}
		}
	}
	if (serialised.contains("tint"))
	{
		auto tintStr = serialised.getString("tint");
		tint = Colour(tintStr);
	}
}

GFXObject::GFXObject() = default;
GFXObject::GFXObject(GFXObject&&) = default;
GFXObject& GFXObject::operator=(GFXObject&&) = default;
GFXObject::~GFXObject()
{
	LOGIF_I(!m_id.empty() && !m_type.empty(), "-- [%s] [%s] destroyed", m_type.data(), m_id.generic_string().data());
}

stdfs::path const& GFXObject::id() const
{
	return m_id;
}

GFXID GFXObject::gfxID() const
{
	return m_glID;
}

bool GFXObject::isReady() const
{
	return m_glID > 0 && context::isAlive();
}

void GFXObject::init(stdfs::path id)
{
	m_id = std::move(id);
	m_type = typeName(*this);
	LOG_I("== [%s] [%s] initialised", m_type.data(), m_id.generic_string().data());
	return;
}

bool GFXObject::preSetup() const
{
	if (!context::isAlive())
	{
		LOG_E("[%s] [Uninitialised] Context not present / is closing!", typeName(*this).data());
		return false;
	}
	if (m_glID > 0)
	{
		LOG_E("[%s] [%s] Already setup!", typeName(*this).data(), m_id.generic_string().data());
		return false;
	}
	return true;
}

bool GFXObject::preDestroy() const
{
	return contextImpl::exists() && m_glID > 0;
}

void UniformBuffer::Descriptor::deserialise(JSONObj const& json)
{
	if (json.contains("id"))
	{
		id = json.getString("id");
		size = (u32)json.getS32("size", (s32)size);
		bindingPoint = (u32)json.getS32("bindingPoint", (s32)bindingPoint);
		auto drawTypeStr = json.getString("drawType");
		utils::strings::toLower(drawTypeStr);
		if (drawTypeStr == "static")
		{
			drawType = DrawType::Static;
		}
		else if (drawTypeStr == "dynamic")
		{
			drawType = DrawType::Dynamic;
		}
	}
	return;
}

UniformBuffer::UniformBuffer() = default;

UniformBuffer::UniformBuffer(Descriptor descriptor)
{
	setup(std::move(descriptor));
}

UniformBuffer::UniformBuffer(UniformBuffer&&) = default;
UniformBuffer& UniformBuffer::operator=(UniformBuffer&&) = default;

UniformBuffer::~UniformBuffer()
{
	if (preDestroy())
	{
		gfx::enqueue([glID = m_glID]() { glChk(glDeleteBuffers(1, &glID.handle)); });
	}
}

bool UniformBuffer::setup(Descriptor descriptor)
{
	m_descriptor = std::move(descriptor);
	if (!preSetup())
	{
		return false;
	}
	GLenum type = m_descriptor.drawType == DrawType::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	gfx::enqueue([this, type, size = m_descriptor.size, bp = m_descriptor.bindingPoint]() {
		LOG_SETUP_ENTER(UniformBuffer, m_id);
		glChk(glGenBuffers(1, &m_glID.handle));
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, m_glID));
		glChk(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, type));
		glChk(glBindBufferBase(GL_UNIFORM_BUFFER, bp, m_glID));
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		LOG_SETUP_EXIT(UniformBuffer, m_id);
	});
	init(std::move(m_descriptor.id));
	return true;
}

void UniformBuffer::copyData(u32 offset, size_t size, void const* pData)
{
	if (isReady())
	{
		std::vector<u8> data(size);
		std::memcpy(data.data(), pData, size);
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, glID = m_glID, offset, size, data = std::move(data)]() {
#else
		gfx::enqueue([glID = m_glID, offset, size, data = std::move(data)]() {
#endif
			LOGIF_X_Y(bDebug, UniformBuffer, "Entered copyData()", glID);
			glChk(glBindBuffer(GL_UNIFORM_BUFFER, glID));
			glChk(glBufferSubData(GL_UNIFORM_BUFFER, (GLintptr)offset, (GLsizeiptr)size, data.data()));
			glChk(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			LOGIF_X_Y(bDebug, UniformBuffer, "Exiting copyData()", glID);
		});
	}
	return;
}

Shader::Shader() = default;

Shader::Shader(Descriptor descriptor)
{
	setup(std::move(descriptor));
}

Shader::Shader(Shader&&) = default;
Shader& Shader::operator=(Shader&&) = default;
Shader::~Shader()
{
	if (preDestroy())
	{
		gfx::enqueue([glID = m_glID]() { glDeleteProgram(glID); });
	}
}

void Shader::bind(GFXID shaderID)
{
	if (g_activeShader != shaderID)
	{
		g_activeShader = shaderID;
		glChk(glUseProgram(g_activeShader));
	}
	return;
}

bool Shader::setup(Descriptor descriptor)
{
	if (!preSetup())
	{
		return false;
	}
	if (descriptor.vertCode.empty())
	{
		LOG_E("[%s] [%s] Failed to compile vertex shader: empty input string!", typeName<Shader>().data(),
			  descriptor.id.generic_string().data());
		return false;
	}
	if (descriptor.fragCode.empty())
	{
		LOG_E("[%s] [%s] Failed to compile fragment shader: empty input string!", typeName<Shader>().data(),
			  descriptor.id.generic_string().data());
		return false;
	}
	gfx::enqueue([this, descriptor]() {
		LOG_SETUP_ENTER(Shader, m_id);
		std::array<char, 512> buf;
		s32 success;
		GLchar const* files[] = {env::g_config.shaderPrefix.data(), "\n", descriptor.vertCode.data()};
		size_t const filesSize = ARR_SIZE(files);
		u32 vsh = glCreateShader(GL_VERTEX_SHADER);
		glChk(glShaderSource(vsh, (GLsizei)filesSize, files, nullptr));
		glChk(glCompileShader(vsh));
		glGetShaderiv(vsh, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vsh, (GLsizei)buf.size(), nullptr, buf.data());
			glDeleteShader(vsh);
			LOG_E("[%s] (Shader) Failed to compile vertex shader!\n\t%s", descriptor.id.generic_string().data(), buf.data());
			return;
		}
		u32 fsh = 0;
		fsh = glCreateShader(GL_FRAGMENT_SHADER);
		files[filesSize - 1] = descriptor.fragCode.data();
		glChk(glShaderSource(fsh, (GLsizei)filesSize, files, nullptr));
		glChk(glCompileShader(fsh));
		glGetShaderiv(fsh, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glDeleteShader(vsh);
			glGetShaderInfoLog(fsh, (GLsizei)buf.size(), nullptr, buf.data());
			glDeleteShader(fsh);
			LOG_E("[%s] (Shader) Failed to compile fragment shader!\n\t%s", descriptor.id.generic_string().data(), buf.data());
			return;
		}
		m_glID = glCreateProgram();
		glChk(glAttachShader(m_glID, vsh));
		glChk(glAttachShader(m_glID, fsh));
		glChk(glLinkProgram(m_glID));
		glGetProgramiv(m_glID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glDeleteShader(vsh);
			glDeleteShader(fsh);
			glGetProgramInfoLog(m_glID, (GLsizei)buf.size(), nullptr, buf.data());
			glDeleteProgram(m_glID);
			LOG_E("[%s] (Shader) Failed to link shader!\n\t%s", descriptor.id.generic_string().data(), buf.data());
			m_glID = {};
			return;
		}
		glDeleteShader(vsh);
		glDeleteShader(fsh);
		for (auto const& uboID : descriptor.uboIDs)
		{
			if (auto pUniformBuffer = GFXStore::instance()->get<UniformBuffer>(uboID))
			{
				auto id = pUniformBuffer->m_id.filename().string();
				id.at(0) = (char)std::toupper(id.at(0));
				u32 idx = glGetUniformBlockIndex(m_glID, id.data());
				if ((s32)idx >= 0)
				{
					glChk(glUniformBlockBinding(m_glID, idx, pUniformBuffer->m_descriptor.bindingPoint));
					LOG_D("[%s] [%s] bound UBO [%s] at [%u]", typeName<Shader>().data(), m_id.generic_string().data(),
						  pUniformBuffer->m_id.generic_string().data(), pUniformBuffer->m_descriptor.bindingPoint);
				}
			}
		}
		LOG_SETUP_EXIT(Shader, m_id);
		return;
	});
	m_flags = descriptor.flags;
	init(std::move(descriptor.id));
	return true;
}

void Shader::use() const
{
	if (isReady())
	{
		gfx::enqueue([glID = m_glID]() { bind(glID); });
	}
	return;
}

void Shader::setBool(std::string_view id, bool bVal) const
{
	if (isReady() && !id.empty())
	{
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, glID = m_glID, id = std::string(id), bVal]() {
#else
		gfx::enqueue([glID = m_glID, id = std::string(id), bVal]() {
#endif
			LOGIF_X_Y(bDebug, Shader, "Entered setBool()", glID);
			bind(glID);
			auto glLoc = glGetUniformLocation(glID, id.data());
			if (glLoc >= 0)
			{
				glChk(glUniform1i(glLoc, (GLint)bVal));
			}
			LOGIF_X_Y(bDebug, Shader, "Exiting setBool()", glID);
			return;
		});
	}
	return;
}

void Shader::setS32(std::string_view id, s32 val) const
{
	if (isReady() && !id.empty())
	{
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, glID = m_glID, id = std::string(id), val]() {
#else
		gfx::enqueue([glID = m_glID, id = std::string(id), val]() {
#endif
			LOGIF_X_Y(bDebug, Shader, "Entered setS32()", glID);
			bind(glID);
			auto glLoc = glGetUniformLocation(glID, id.data());
			if (glLoc >= 0)
			{
				glChk(glUniform1i(glLoc, (GLint)val));
			}
			LOGIF_X_Y(bDebug, Shader, "Exiting setS32()", glID);
			return;
		});
	}
	return;
}
void Shader::setF32(std::string_view id, f32 val) const
{
	if (isReady() && !id.empty())
	{
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, glID = m_glID, id = std::string(id), val]() {
#else
		gfx::enqueue([glID = m_glID, id = std::string(id), val]() {
#endif
			LOGIF_X_Y(bDebug, Shader, "Entered setF32()", glID);
			bind(glID);
			auto glLoc = glGetUniformLocation(glID, id.data());
			if (glLoc >= 0)
			{
				glChk(glUniform1f(glLoc, val));
			}
			LOGIF_X_Y(bDebug, Shader, "Exiting setF32()", glID);
			return;
		});
	}
	return;
}

void Shader::setV2(std::string_view id, glm::vec2 const& val) const
{
	if (isReady() && !id.empty())
	{
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, glID = m_glID, id = std::string(id), val = val]() {
#else
		gfx::enqueue([glID = m_glID, id = std::string(id), val = val]() {
#endif
			LOGIF_X_Y(bDebug, Shader, "Entered setV2()", glID);
			bind(glID);
			auto glLoc = glGetUniformLocation(glID, id.data());
			if (glLoc >= 0)
			{
				glChk(glUniform2f(glLoc, val.x, val.y));
			}
			LOGIF_X_Y(bDebug, Shader, "Exiting setV2()", glID);
		});
		return;
	}
	return;
}

void Shader::setV3(std::string_view id, glm::vec3 const& val) const
{
	if (isReady() && !id.empty())
	{
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, glID = m_glID, id = std::string(id), val = val]() {
#else
		gfx::enqueue([glID = m_glID, id = std::string(id), val = val]() {
#endif
			LOGIF_X_Y(bDebug, Shader, "Entered setV3()", glID);
			bind(glID);
			auto glLoc = glGetUniformLocation(glID, id.data());
			if (glLoc >= 0)
			{
				glChk(glUniform3f(glLoc, val.x, val.y, val.z));
			}
			LOGIF_X_Y(bDebug, Shader, "Exiting setV3()", glID);
		});
		return;
	}
	return;
}

void Shader::setV4(std::string_view id, Colour colour) const
{
	return setV4(id, glm::vec4(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
}

void Shader::setV4(std::string_view id, glm::vec4 const& val) const
{
	if (isReady() && !id.empty())
	{
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, glID = m_glID, id = std::string(id), val = val]() {
#else
		gfx::enqueue([glID = m_glID, id = std::string(id), val = val]() {
#endif
			LOGIF_X_Y(bDebug, Shader, "Entered setV4()", glID);
			bind(glID);
			auto glLoc = glGetUniformLocation(glID, id.data());
			if (glLoc >= 0)
			{
				glChk(glUniform4f(glLoc, val.x, val.y, val.z, val.w));
			}
			LOGIF_X_Y(bDebug, Shader, "Exiting setV4()", glID);
		});
		return;
	}
	return;
}

void Shader::setModelMats(ModelMats const& mats) const
{
	if (isReady())
	{
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, glID = m_glID, mats = mats]() {
#else
		gfx::enqueue([glID = m_glID, mats = mats]() {
#endif
			LOGIF_X_Y(bDebug, Shader, "Exiting setModelMats()", glID);
			bind(glID);
			auto temp = glGetUniformLocation(glID, env::g_config.uniforms.modelMatrix.data());
			glChk(glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(mats.model)));
			temp = glGetUniformLocation(glID, env::g_config.uniforms.normalMatrix.data());
			glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(mats.normals));
			LOGIF_X_Y(bDebug, Shader, "Exiting setModelMats()", glID);
			return;
		});
	}
	return;
}

void Shader::setMaterial(Material const& material) const
{
	if (isReady())
	{
		auto const& u = env::g_config.uniforms;
		setV4(u.material.tint, material.tint);
		bool const bIsLit = material.flags.isSet(Material::Flag::Lit);
		setBool(u.material.isLit, bIsLit);
		bool const bIsTextured = material.flags.isSet(Material::Flag::Textured);
		setBool(u.material.isTextured, bIsTextured);
		if (bIsLit)
		{
			setF32(u.material.albedo.shininess, material.albedo.shininess);
			setV3(u.material.albedo.ambient, material.albedo.ambient);
			setV3(u.material.albedo.diffuse, material.albedo.diffuse);
			setV3(u.material.albedo.specular, material.albedo.specular);
		}
		if (bIsTextured)
		{
			setBool(u.material.isOpaque, material.flags.isSet(Material::Flag::Opaque));
			setBool(u.material.isFont, material.flags.isSet(Material::Flag::Font));
			setF32(u.material.hasSpecular, material.flags.isSet(Material::Flag::Specular) ? 1.0f : 0.0f);
		}
	}
	return;
}

void Shader::bind(UniformBuffer const& ubo) const
{
	if (isReady() && !ubo.m_id.empty() && ubo.m_descriptor.bindingPoint > 0)
	{
		LOG_D("[%s] [%s] bound UBO [%s] at [%u]", typeName<Shader>().data(), m_id.generic_string().data(), ubo.m_id.generic_string().data(),
			  ubo.m_descriptor.bindingPoint);
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, glID = m_glID, id = ubo.m_id, bp = ubo.m_descriptor.bindingPoint]() {
#else
		gfx::enqueue([glID = m_glID, id = ubo.m_id, bp = ubo.m_descriptor.bindingPoint]() {
#endif
			u32 idx = glGetUniformBlockIndex(glID, id.generic_string().data());
			if ((s32)idx >= 0)
			{
				LOGIF_X_Y(bDebug, Shader, "Entered bind(UniformBuffer)", glID);
				glChk(glUniformBlockBinding(glID, idx, bp));
				LOGIF_X_Y(bDebug, Shader, "Exiting bind(UniformBuffer)", glID);
			}
			return;
		});
	}
	return;
}

void Shader::bind(std::initializer_list<Texture const*> textures) const
{
	if (isReady())
	{
		auto const& u = env::g_config.uniforms;
		auto const pBlank = GFXStore::instance()->m_pBlankTexture;
		for (auto pTexture : textures)
		{
			ASSERT(pTexture, "Passed texture is null!");
			if (!pTexture)
			{
				continue;
			}
			if (!pTexture->isReady() && pTexture->m_descriptor.type == TexType::Diffuse && pBlank)
			{
				pTexture = pBlank;
				setV4("tint", colours::Magenta);
			}
			if (pTexture->isReady())
			{
				std::string_view uniformName;
				auto const type = pTexture->m_descriptor.type;
				switch (type)
				{
				default:
				case TexType::Diffuse:
					uniformName = u.material.diffuseTexPrefix;
					break;
				case TexType::Specular:
					uniformName = u.material.specularTexPrefix;
					break;
				}
				GFXID diffuseSampler = pTexture->m_descriptor.pSampler ? pTexture->m_descriptor.pSampler->m_glID : GFXID();
				setTexture((s32)type, diffuseSampler, pTexture->m_glID);
				setS32(uniformName, (s32)type);
			}
		}
	}
}

void Shader::bind(std::vector<Texture const*> const& textures) const
{
	if (isReady())
	{
		auto const& u = env::g_config.uniforms;
		auto const pBlank = GFXStore::instance()->m_pBlankTexture;
		for (auto pTexture : textures)
		{
			ASSERT(pTexture, "Passed texture is null!");
			if (!pTexture)
			{
				continue;
			}
			if (!pTexture->isReady() && pTexture->m_descriptor.type == TexType::Diffuse && pBlank)
			{
				pTexture = pBlank;
				setV4("tint", colours::Magenta);
			}
			if (pTexture->isReady())
			{
				std::string_view uniformName;
				auto const type = pTexture->m_descriptor.type;
				switch (type)
				{
				default:
				case TexType::Diffuse:
					uniformName = u.material.diffuseTexPrefix;
					break;
				case TexType::Specular:
					uniformName = u.material.specularTexPrefix;
					break;
				}
				GFXID diffuseSampler = pTexture->m_descriptor.pSampler ? pTexture->m_descriptor.pSampler->m_glID : GFXID();
				setTexture((s32)type, diffuseSampler, pTexture->m_glID);
				setS32(uniformName, (s32)type);
			}
		}
	}
}

void Shader::unbind(std::initializer_list<TexType> units) const
{
	if (isReady())
	{
		auto const& u = env::g_config.uniforms;
		for (auto type : units)
		{
			std::string id;
			switch (type)
			{
			default:
			case TexType::Diffuse:
				id = u.material.diffuseTexPrefix;
				break;
			case TexType::Specular:
				id = u.material.specularTexPrefix;
				break;
			}
			setTexture((s32)type, 0, 0);
			setS32(id, (s32)type);
		}
		setBool(u.material.isTextured, false);
	}
}

VertexArray::VertexArray() = default;

VertexArray::VertexArray(Descriptor descriptor, Geometry geometry)
{
	setup(std::move(descriptor), std::move(geometry));
}

VertexArray::VertexArray(VertexArray&&) = default;
VertexArray& VertexArray::operator=(VertexArray&&) = default;
VertexArray::~VertexArray()
{
	if (preDestroy())
	{
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([id = m_id, vao = m_glID, ebo = m_ebo, vbo = m_geometryVBO, instanceVBO = m_instanceVBO]() {
#else
		gfx::enqueue([vao = m_glID, ebo = m_ebo, vbo = m_geometryVBO, instanceVBO = m_instanceVBO]() {
#endif
			LOGIF_X_Y(true, VertexArray, "Entered ~dtor()", vao);
			glChk(glDeleteVertexArrays(1, &vao.handle));
			glChk(glDeleteBuffers(1, &ebo.handle));
			glChk(glDeleteBuffers(1, &vbo.handle));
			glChk(glDeleteBuffers(1, &instanceVBO.handle));
			LOGIF_X_Y(true, VertexArray, "Exiting ~dtor()", vao);
			return;
		});
	}
}

bool VertexArray::setup(Descriptor descriptor, Geometry geometry)
{
	if (!preSetup())
	{
		return false;
	}
	m_descriptor = std::move(descriptor);
	if (geometry.points.empty())
	{
		gfx::enqueue([this]() {
			LOG_SETUP_ENTER(VertexArray, m_id);
			glChk(glGenVertexArrays(1, &m_glID.handle));
			glChk(glGenBuffers(1, &m_geometryVBO.handle));
			glChk(glGenBuffers(1, &m_ebo.handle));
			glChk(glGenBuffers(1, &m_instanceVBO.handle));
			LOG_SETUP_EXIT(VertexArray, m_id);
			return;
		});
	}
	else
	{
		ASSERT(geometry.normals.empty() || geometry.normals.size() == geometry.points.size(), "Point/normal count mismatch!");
		ASSERT(geometry.texCoords.empty() || geometry.texCoords.size() == geometry.points.size(), "Point/UV count mismatch!");
		m_vertexCount = geometry.vertexCount();
		m_indexCount = (u32)geometry.indices.size();
		gfx::enqueue([this, geometry = std::move(geometry)]() {
			LOG_SETUP_ENTER(VertexArray, m_id);
			glChk(glGenVertexArrays(1, &m_glID.handle));
			glChk(glGenBuffers(1, &m_geometryVBO.handle));
			glChk(glGenBuffers(1, &m_ebo.handle));
			glChk(glGenBuffers(1, &m_instanceVBO.handle));
			setGeometryAttributes(std::move(geometry), m_glID, m_geometryVBO, m_ebo, m_descriptor.drawType);
			glChk(glBindVertexArray(0));
			LOG_SETUP_EXIT(VertexArray, m_id);
			return;
		});
	}
	init(std::move(m_descriptor.id));
	return true;
}

void VertexArray::updateGeometry(Geometry geometry)
{
	if (isReady())
	{
		ASSERT(geometry.normals.empty() || geometry.normals.size() == geometry.points.size(), "Point/normal count mismatch!");
		ASSERT(geometry.texCoords.empty() || geometry.texCoords.size() == geometry.points.size(), "Point/UV count mismatch!");
		m_vertexCount = geometry.vertexCount();
		m_indexCount = (u32)geometry.indices.size();
#if defined(LE3D_GFX_DEBUG_LOGS)
		gfx::enqueue([bDebug = m_bDEBUG, geometry = std::move(geometry), vao = m_glID, vbo = m_geometryVBO, ebo = m_ebo,
					  type = m_descriptor.drawType]() {
#else
		gfx::enqueue([geometry = std::move(geometry), vao = m_glID, vbo = m_geometryVBO, ebo = m_ebo, type = m_descriptor.drawType]() {
#endif
			LOGIF_X_Y(bDebug, VertexArray, "Entered updateGeometry()", vao);
			setGeometryAttributes(std::move(geometry), vao, vbo, ebo, type);
			LOGIF_X_Y(bDebug, VertexArray, "Exiting updateGeometry()", vao);
			return;
		});
	}
	return;
}

void VertexArray::setInstances(InstanceBuffer instances)
{
	ASSERT(isReady(), "VertexArray not set up!");
	if (isReady())
	{
		m_instanceCount = instances.instanceCount();
		if (m_instanceCount > 0)
		{
#if defined(LE3D_GFX_DEBUG_LOGS)
			gfx::enqueue([bDebug = m_bDEBUG, instances = std::move(instances), glID = m_glID, vbo = m_instanceVBO]() {
#else
			gfx::enqueue([instances = std::move(instances), glID = m_glID, vbo = m_instanceVBO]() {
#endif
				LOGIF_X_Y(bDebug, VertexArray, "Entered setInstances()", glID);
				GLint constexpr vaSize = 4;
				auto constexpr vaBytes = sizeof(glm::mat4);
				GLboolean constexpr glNorm = GL_FALSE;
				u32 const vec4sPerAttrib = vaSize;
				GLenum glDrawType = instances.drawType == DrawType::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
				auto const size = instances.models.size() * vaBytes;
				glChk(glBindBuffer(GL_ARRAY_BUFFER, vbo));
				glChk(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)size, instances.models.data(), glDrawType));
				glChk(glBindVertexArray(glID));
				for (u32 idx = 0; idx < vec4sPerAttrib; ++idx)
				{
					auto offset = idx * sizeof(glm::vec4);
					u32 loc = u32(s_instanceAttribLoc) + idx;
					glChk(glVertexAttribPointer(loc, vaSize, GL_FLOAT, glNorm, vaBytes, (void*)offset));
					glChk(glEnableVertexAttribArray(loc));
					glChk(glVertexAttribDivisor(loc, 1));
				}
				glChk(glBindBuffer(GL_ARRAY_BUFFER, 0));
				glChk(glBindVertexArray(0));
				LOGIF_X_Y(bDebug, VertexArray, "Exiting setInstances()", glID);
				return;
			});
		}
	}
}

void VertexArray::draw(Shader const& shader) const
{
	if (isReady() && shader.isReady())
	{
		shader.setBool(env::g_config.uniforms.transform.isInstanced, m_instanceCount > 0);
#if defined(LE3D_GFX_DEBUG_LOGS)
		auto drawArrays = [bDebug = m_bDEBUG, id = m_id, vao = m_glID, shaderID = shader.gfxID(), instanceCount = m_instanceCount,
						   vCount = m_vertexCount]() {
#else
		auto drawArrays = [vao = m_glID, shaderID = shader.gfxID(), instanceCount = m_instanceCount, vCount = m_vertexCount]() {
#endif
			LOGIF_X_Y(bDebug, VertexArray, "Entered draw()", vao);
			glChk(glBindVertexArray(vao));
			if (instanceCount > 0)
			{
				glChk(glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)vCount, (GLsizei)instanceCount));
			}
			else
			{
				glChk(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vCount));
			}
			LOGIF_X_Y(bDebug, VertexArray, "Exiting draw()", vao);
			return;
		};
#if defined(LE3D_GFX_DEBUG_LOGS)
		auto drawElements = [bDebug = m_bDEBUG, id = m_id, vao = m_glID, shaderID = shader.gfxID(), instanceCount = m_instanceCount,
							 iCount = m_indexCount]() {
#else
		auto drawElements = [vao = m_glID, shaderID = shader.gfxID(), instanceCount = m_instanceCount, iCount = m_indexCount]() {
#endif
			LOGIF_X_Y(bDebug, VertexArray, "Entered draw()", vao);
			glChk(glBindVertexArray(vao));
			if (instanceCount > 0)
			{
				glChk(glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)iCount, GL_UNSIGNED_INT, 0, (GLsizei)instanceCount));
			}
			else
			{
				glChk(glDrawElements(GL_TRIANGLES, (GLsizei)iCount, GL_UNSIGNED_INT, 0));
			}
			LOGIF_X_Y(bDebug, VertexArray, "Exiting draw()", vao);
			return;
		};
		if (m_ebo > 0 && m_indexCount > 0)
		{
			gfx::enqueue(drawElements);
		}
		else
		{
			gfx::enqueue(drawArrays);
		}
	}
	return;
}

void VertexArray::setGeometryAttributes(Geometry geometry, GFXID vao, GFXID vbo, GFXID ebo, DrawType type)
{
	glChk(glBindVertexArray(vao));
	glChk(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GLenum glType = type == DrawType::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	glChk(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)geometry.byteCount(), nullptr, glType));
	auto constexpr sv3 = (size_t)sizeof(Geometry::V3);
	auto constexpr sv2 = (size_t)sizeof(Geometry::V2);
	auto const& p = geometry.points;
	auto const& n = geometry.normals;
	auto const& t = geometry.texCoords;
	glChk(glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(sv3 * p.size()), p.data()));
	glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sv3 * p.size()), (GLsizeiptr)(sv3 * n.size()), n.data()));
	glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sv3 * (p.size() + n.size())), (GLsizeiptr)(sv2 * t.size()), t.data()));
	if (!geometry.indices.empty())
	{
		GLsizeiptr size = GLsizeiptr(geometry.indices.size() * sizeof(u32));
		glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
		glChk(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, geometry.indices.data(), glType));
	}
	auto constexpr sf = (size_t)sizeof(f32);
	// Position		: 3x vec3
	GLint loc = 0;
	glChk(glVertexAttribPointer((GLuint)loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)(sv3), 0));
	glChk(glEnableVertexAttribArray((GLuint)loc));
	// Normal		: 3x vec3
	loc = 1;
	glChk(glVertexAttribPointer((GLuint)loc, 3, GL_FLOAT, GL_FALSE, (GLsizei)(sv3), (void*)(sv3 * p.size())));
	glChk(glEnableVertexAttribArray((u32)loc));
	// Tex coord	: 2x vec2
	loc = 2;
	glChk(glVertexAttribPointer((GLuint)loc, 2, GL_FLOAT, GL_FALSE, (GLsizei)(2 * sf), (void*)(sv3 * (p.size() + n.size()))));
	glChk(glEnableVertexAttribArray((u32)loc));
	return;
}

void Sampler::Descriptor::deserialise(JSONObj const& json)
{
	if (json.contains("id"))
	{
		id = json.getString("id");
		auto wrapStr = json.getString("wrap", "repeat");
		auto minFilterStr = json.getString("minFilter", "linearmplinear");
		auto magFilterStr = json.getString("magFilter", "linear");
		anisotropy = (u8)json.getS32("anisotropy", 4);
		utils::strings::toLower(wrapStr);
		utils::strings::toLower(minFilterStr);
		utils::strings::toLower(magFilterStr);
		wrap = texWrap(wrapStr);
		minFilter = texFilter(minFilterStr);
		magFilter = texFilter(magFilterStr);
	}
}

Sampler::Sampler() = default;

Sampler::Sampler(Descriptor descriptor)
{
	setup(std::move(descriptor));
}

Sampler::Sampler(Sampler&&) = default;
Sampler& Sampler::operator=(Sampler&&) = default;

Sampler::~Sampler()
{
	if (preDestroy())
	{
		gfx::enqueue([glID = m_glID]() { glDeleteSamplers(1, &glID.handle); });
	}
}

bool Sampler::setup(Descriptor descriptor)
{
	if (!preSetup())
	{
		return false;
	}
	m_descriptor = std::move(descriptor);
	GLint glWrap;
	switch (m_descriptor.wrap)
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
	GLint glMinFilter = getGLFilter(m_descriptor.minFilter);
	GLint glMagFilter = getGLFilter(m_descriptor.magFilter);
	gfx::enqueue([this, glWrap, glMinFilter, glMagFilter, aniso = m_descriptor.anisotropy]() {
		LOG_SETUP_ENTER(Sampler, m_id);
		glChk(glGenSamplers(1, &m_glID.handle));
		glChk(glSamplerParameteri(m_glID, GL_TEXTURE_WRAP_S, glWrap));
		glChk(glSamplerParameteri(m_glID, GL_TEXTURE_WRAP_T, glWrap));
		glChk(glSamplerParameteri(m_glID, GL_TEXTURE_MIN_FILTER, glMinFilter));
		glChk(glSamplerParameteri(m_glID, GL_TEXTURE_MAG_FILTER, glMagFilter));
#if defined(LE3D_USE_GLAD) && defined(GL_ARB_texture_filter_anisotropic)
		if (GLAD_GL_ARB_texture_filter_anisotropic == 1)
		{
			glChk(glSamplerParameteri(m_glID, GL_TEXTURE_MAX_ANISOTROPY, aniso));
		}
#endif
		LOG_SETUP_EXIT(Sampler, m_id);
		return;
	});
	init(std::move(m_descriptor.id));
	return true;
}

Sampler::Descriptor const& Sampler::descriptor() const
{
	return m_descriptor;
}

void Texture::Descriptor::deserialise(JSONObj const& json)
{
	if (json.contains("id"))
	{
		id = json.getString("id");
		samplerID = json.getString("samplerID");
		auto typeStr = json.getString("type", "diffuse");
		utils::strings::toLower(typeStr);
		type = texType(typeStr);
	}
}

Texture::Texture() = default;

Texture::Texture(Descriptor descriptor, bytearray image)
{
	setup(std::move(descriptor), std::move(image));
}

Texture::Texture(Texture&&) = default;
Texture& Texture::operator=(Texture&&) = default;

Texture::~Texture()
{
	if (preDestroy())
	{
		gfx::enqueue([glID = m_glID]() { glDeleteTextures(1, &glID.handle); });
	}
}

bool Texture::setup(Descriptor descriptor, bytearray texBytes, u8 ch, u16 w, u16 h)
{
	if (!preSetup())
	{
		return false;
	}
	m_descriptor = std::move(descriptor);
	m_descriptor.size = {w, h};
	if (!m_descriptor.samplerID.empty())
	{
		m_descriptor.pSampler = GFXStore::instance()->get<Sampler>(m_descriptor.samplerID);
	}
	bool const bAlpha = ch > 3;
	gfx::enqueue([this, texBytes = std::move(texBytes), bAlpha, w, h]() {
		LOG_SETUP_ENTER(Texture, m_id);
		glChk(glGenTextures(1, &m_glID.handle));
		g_activeTextureSampler[0] = {0, 0};
		glChk(glActiveTexture(GL_TEXTURE0));
		glChk(glBindTexture(GL_TEXTURE_2D, m_glID));
		glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		glChk(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLint const extFormat = bAlpha ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB;
		GLenum const intFormat = bAlpha ? GL_RGBA : GL_RGB;
		glChk(glTexImage2D(GL_TEXTURE_2D, 0, extFormat, w, h, 0, intFormat, GL_UNSIGNED_BYTE, texBytes.data()));
		glChk(glGenerateMipmap(GL_TEXTURE_2D));
		glChk(glBindTexture(GL_TEXTURE_2D, 0));
		LOG_SETUP_EXIT(Texture, m_id);
		return;
	});
	init(std::move(m_descriptor.id));
	return true;
}

bool Texture::setup(Descriptor descriptor, bytearray image)
{
	s32 w, h, ch;
	bool bRet;
	stbi_uc* pData = nullptr;
	{
		Lock lock(g_stbiMutex);
		stbi_set_flip_vertically_on_load(1);
		pData = stbi_load_from_memory(reinterpret_cast<u8 const*>(image.data()), (s32)image.size(), &w, &h, &ch, 0);
	}
	if (pData)
	{
		size_t size = size_t(w * h * ch);
		bytearray texBytes(size);
		std::memcpy(texBytes.data(), pData, size);
		bRet = setup(std::move(descriptor), std::move(texBytes), (u8)ch, (u16)w, (u16)h);
	}
	else
	{
		LOG_E("[%s] Failed to load texture: [%s]!", typeName<Texture>().data(), descriptor.id.generic_string().data());
		bRet = false;
	}
	stbi_image_free(pData);
	return bRet;
}

void Texture::setSampler(Sampler const* pSampler)
{
	m_descriptor.pSampler = pSampler;
	return;
}

Texture::Descriptor const& Texture::descriptor() const
{
	return m_descriptor;
}

void Font::Glyph::deserialise(u8 c, JSONObj const& json)
{
	ch = c;
	st = {json.getS32("x", 0), json.getS32("y", 0)};
	cell = {json.getS32("width", 0), json.getS32("height", 0)};
	uv = cell;
	offset = {json.getS32("originX", 0), json.getS32("originY", 0)};
	xAdv = json.getS32("advance", cell.x);
	orgSizePt = json.getS32("size", 0);
	bBlank = json.getBool("isBlank", false);
}

bool Font::Descriptor::deserialise(JSONObj const& json)
{
	if (json.contains("id"))
	{
		id = json.getString("id");
		sheetID = json.getString("sheetID");
		samplerID = json.getString("sampler", "font");
		auto glyphsData = json.getGData("glyphs");
		for (auto const kvp : glyphsData.allFields())
		{
			if (!kvp.first.empty())
			{
				Glyph glyph;
				glyph.deserialise((u8)kvp.first.at(0), GData(kvp.second));
				if (glyph.cell.x > 0 && glyph.cell.y > 0)
				{
					glyphs.push_back(std::move(glyph));
				}
				else
				{
					LOG_W("[%s] [%s] Could not deserialise Glyph '%c'!", typeName<Font>().data(), id.generic_string().data(),
						  kvp.first.at(0));
				}
			}
		}
		if (json.contains("material"))
		{
			material.deserialise(json.getGData("material"));
		}
		return true;
	}
	return false;
}

GFXID Font::s_nextID = 0;

Font::Font() = default;

Font::Font(Descriptor descriptor, bytearray image)
{
	setup(std::move(descriptor), std::move(image));
}

Font::Font(Font&&) = default;
Font& Font::operator=(Font&&) = default;

Font::~Font() = default;

bool Font::setup(Descriptor descriptor, bytearray image)
{
	if (!preSetup())
	{
		return false;
	}
	Texture::Descriptor sheetDesc;
	sheetDesc.id += descriptor.id;
	sheetDesc.id += "_sheet";
	sheetDesc.samplerID = descriptor.samplerID;
	sheetDesc.type = TexType::Diffuse;
	if (!m_sheet.setup(std::move(sheetDesc), std::move(image)))
	{
		return false;
	}
	glm::ivec2 maxCell = glm::vec2(0);
	s32 maxXAdv = 0;
	for (auto const& glyph : descriptor.glyphs)
	{
		ASSERT(glyph.ch != '\0' && m_glyphs[(size_t)glyph.ch].ch == '\0', "Invalid/duplicate glyph!");
		m_glyphs.at((size_t)glyph.ch) = glyph;
		maxCell.x = std::max(maxCell.x, glyph.cell.x);
		maxCell.y = std::max(maxCell.y, glyph.cell.y);
		maxXAdv = std::max(maxXAdv, glyph.xAdv);
		if (glyph.bBlank)
		{
			m_blankGlyph = glyph;
		}
	}
	if (m_blankGlyph.xAdv == 0)
	{
		m_blankGlyph.cell = maxCell;
		m_blankGlyph.xAdv = maxXAdv;
	}
	m_material = descriptor.material;
	m_material.flags.set({Material::Flag::Textured, Material::Flag::Font}, true);
	m_material.flags.set(Material::Flag::Opaque, false);
	gfx::enqueue([this]() { m_glID = ++s_nextID.handle; });
	init(std::move(descriptor.id));
	return true;
}

Geometry Font::generate(Text const& text) const
{
	if (text.text.empty() || !isReady())
	{
		return {};
	}
	glm::ivec2 maxCell = glm::vec2(0);
	for (auto c : text.text)
	{
		maxCell.x = std::max(maxCell.x, m_glyphs.at((size_t)c).cell.x);
		maxCell.y = std::max(maxCell.y, m_glyphs.at((size_t)c).cell.y);
	}
	u32 lineCount = 1;
	for (size_t idx = 0; idx < text.text.size(); ++idx)
	{
		if (text.text[idx] == '\n')
		{
			++lineCount;
		}
	}
	f32 const lineHeight = ((f32)maxCell.y) * text.scale;
	f32 const linePad = lineHeight * text.nYPad;
	f32 const textHeight = lineCount * lineHeight;
	glm::vec2 const realTopLeft = text.pos;
	glm::vec2 textTL = realTopLeft;
	size_t nextLineIdx = 0;
	s32 yIdx = 0;
	f32 xPos = 0.0f;
	f32 lineWidth = 0.0f;
	auto const textTLoffset = getTextTLOffset(text.halign, text.valign);
	auto updateTextTL = [&]() {
		lineWidth = 0.0f;
		for (; nextLineIdx < text.text.size(); ++nextLineIdx)
		{
			auto const ch = text.text.at(nextLineIdx);
			if (ch == '\n')
			{
				break;
			}
			else
			{
				lineWidth += m_glyphs.at((size_t)ch).xAdv;
			}
		}
		lineWidth *= text.scale;
		++nextLineIdx;
		xPos = 0.0f;
		textTL = realTopLeft + textTLoffset * glm::vec2(lineWidth, textHeight);
		textTL.y -= (lineHeight + (yIdx * (lineHeight + linePad)));
	};
	updateTextTL();

	Geometry verts;
	u32 quadCount = (u32)text.text.length();
	verts.reserve(4 * quadCount, 6 * quadCount);
	auto const normal = glm::vec3(0.0f);
	auto const texSize = m_sheet.descriptor().size;
	for (auto const c : text.text)
	{
		if (c == '\n')
		{
			++yIdx;
			updateTextTL();
			continue;
		}
		auto const& search = m_glyphs.at((size_t)c);
		auto const& glyph = search.ch == '\0' ? m_blankGlyph : search;
		auto const offset = glm::vec3(xPos - glyph.offset.x * text.scale, glyph.offset.y * text.scale, 0.0f);
		auto const tl = glm::vec3(textTL.x, textTL.y, text.pos.z) + offset;
		auto const s = (f32)glyph.st.x / texSize.x;
		auto const t = 1.0f - (f32)glyph.st.y / texSize.y;
		auto const u = s + (f32)glyph.uv.x / texSize.x;
		auto const v = t - (f32)glyph.uv.y / texSize.y;
		glm::vec2 const cell = {glyph.cell.x * text.scale, glyph.cell.y * text.scale};
		auto const v0 = verts.addVertex(tl, normal, glm::vec2(s, t));
		auto const v1 = verts.addVertex(tl + glm::vec3(cell.x, 0.0f, 0.0f), normal, glm::vec2(u, t));
		auto const v2 = verts.addVertex(tl + glm::vec3(cell.x, -cell.y, 0.0f), normal, glm::vec2(u, v));
		auto const v3 = verts.addVertex(tl + glm::vec3(0.0f, -cell.y, 0.0f), normal, glm::vec2(s, v));
		verts.addIndices({v0, v1, v2, v2, v3, v0});
		xPos += (glyph.xAdv * text.scale);
	}
	return verts;
}

Cubemap::Cubemap() = default;

Cubemap::Cubemap(Descriptor descriptor, std::array<bytearray, 6> rludfb)
{
	setup(std::move(descriptor), std::move(rludfb));
}

Cubemap::Cubemap(Cubemap&&) = default;
Cubemap& Cubemap::operator=(Cubemap&&) = default;

Cubemap::~Cubemap()
{
	if (preDestroy())
	{
		gfx::enqueue([glID = m_glID]() { glChk(glDeleteTextures(1, &glID.handle)); });
	}
}

bool Cubemap::setup(Descriptor descriptor, std::array<bytearray, 6> rludfb)
{
	if (!preSetup())
	{
		return false;
	}
	gfx::enqueue([this, rludfb = std::move(rludfb)]() {
		LOG_SETUP_ENTER(Cubemap, m_id);
		glChk(glGenTextures(1, &m_glID.handle));
		glChk(glBindTexture(GL_TEXTURE_CUBE_MAP, m_glID));
		s32 w, h, ch;
		u32 idx = 0;
		for (auto const& side : rludfb)
		{
			stbi_uc* pData = nullptr;
			{
				Lock lock(g_stbiMutex);
				stbi_set_flip_vertically_on_load(0);
				pData = stbi_load_from_memory(reinterpret_cast<u8 const*>(side.data()), (s32)side.size(), &w, &h, &ch, 0);
			}
			if (pData)
			{
				bool bAlpha = ch > 3;
				s32 channels = bAlpha ? GL_RGBA : GL_RGB;
				glChk(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, 0, channels, w, h, 0, (u32)channels, GL_UNSIGNED_BYTE, pData));
				glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
				glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
				glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
				glChk(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
			}
			else
			{
				LOG_E("Failed to load cubemap texture #%d!", idx);
			}
			++idx;
			stbi_image_free(pData);
		}
		LOG_SETUP_ENTER(Cubemap, m_id);
		return;
	});
	init(std::move(descriptor.id));
	return true;
}

GFXID Skybox::s_nextID = 0;

void Skybox::Descriptor::deserialise(JSONObj const& json)
{
	if (json.contains("id"))
	{
		id = json.getString("id");
		cubemapID = json.getString("cubemapID");
		cubeVertsID = json.getString("cubeVertsID");
		shaderID = json.getString("shaderID");
	}
	return;
}

bool Skybox::setup(Descriptor descriptor)
{
	if (!preSetup())
	{
		return false;
	}
	auto pStore = GFXStore::instance();
	m_pCubemap = pStore->get<Cubemap>(descriptor.cubemapID);
	m_pCube = pStore->get<VertexArray>(descriptor.cubeVertsID);
	m_pShader = pStore->get<Shader>(descriptor.shaderID);
	if (m_pCubemap && m_pCube && m_pShader)
	{
		gfx::enqueue([this]() { m_glID = ++s_nextID.handle; });
		init(std::move(descriptor.id));
		return true;
	}
	return false;
}

void Skybox::setCubemap(Cubemap const& cubemap)
{
	ASSERT(&cubemap, "Null reference!");
	if (!cubemap.isReady())
	{
		LOG_W("[%s] Dependency is not ready! [%s]: [%s]", typeName<Cubemap>().data(), typeName(cubemap).data(),
			  cubemap.id().generic_string().data());
		return;
	}
	m_pCubemap = &cubemap;
	return;
}

void Skybox::setShader(Shader const& shader)
{
	ASSERT(&shader, "Null reference!");
	if (!shader.isReady())
	{
		LOG_W("[%s] Dependency is not ready! [%s]: [%s]", typeName<Cubemap>().data(), typeName(shader).data(),
			  shader.id().generic_string().data());
		return;
	}
	m_pShader = &shader;
	return;
}

void Skybox::render(Colour tint /* = colours::White */)
{
	if (!allReady())
	{
		return;
	}
	LOGIF_W(!m_pShader->m_flags.isSet(Shader::Flag::Skybox), "[%s] Render(): Shader Skybox flag not set: [%s]!", m_type.data(),
			m_pShader->id().generic_string().data());
	auto const& u = env::g_config.uniforms;
	m_pShader->setV4(u.material.tint, tint);
	m_pShader->setS32(u.material.skybox, s_unitID);
	g_activeTextureSampler[s_unitID] = {m_pCubemap->gfxID(), 0};
#if defined(LE3D_GFX_DEBUG_LOGS)
	gfx::enqueue([bDebug = m_bDEBUG, pShader = m_pShader, pCube = m_pCube, pCubemap = m_pCubemap]() {
#else
	gfx::enqueue([pShader = m_pShader, pCube = m_pCube, pCubemap = m_pCubemap]() {
#endif
		LOGIF_X_Y(bDebug, Skybox, "Entered render()", pCubemap->gfxID());
		Shader::bind(pShader->gfxID());
		glChk(glDepthMask(GL_FALSE));
		glChk(glActiveTexture(GL_TEXTURE0 + s_unitID));
		glChk(glBindSampler(0, 0));
		glChk(glBindVertexArray(pCube->gfxID()));
		glChk(glBindTexture(GL_TEXTURE_CUBE_MAP, pCubemap->gfxID()));
		LOGIF_X_Y(bDebug, Skybox, "Exiting render()", pCubemap->gfxID());
	});
	m_pCube->draw(*m_pShader);
	gfx::enqueue([]() {
		glChk(glBindVertexArray(0));
		glChk(glDepthMask(GL_TRUE));
	});
	return;
}

VertexArray const* Skybox::vertices() const
{
	return m_pCube;
}

bool Skybox::allReady() const
{
	return m_pCube && m_pCube->isReady() && m_pCubemap && m_pCubemap->isReady() && m_pShader && m_pShader->isReady();
}

Mesh::Mesh() = default;
Mesh::Mesh(Mesh&&) = default;

Mesh::Mesh(Descriptor descriptor)
{
	setup(std::move(descriptor));
}

Mesh& Mesh::operator=(Mesh&&) = default;
Mesh::~Mesh() = default;

GFXID Mesh::s_nextID = 0;

bool Mesh::setup(Descriptor descriptor)
{
	if (!preSetup())
	{
		return false;
	}
	m_geometry = std::move(descriptor.geometry);
	m_material = std::move(descriptor.material);
	VertexArray::Descriptor desc;
	desc.id = descriptor.id;
	m_drawType = desc.drawType = descriptor.drawType;
	if (m_verts.setup(std::move(desc), m_geometry))
	{
		init(std::move(descriptor.id));
		gfx::enqueue([this]() { m_glID = ++s_nextID.handle; });
		return true;
	}
	return false;
}

bool Mesh::regenerate(Geometry geometry)
{
	if (isReady() && m_verts.isReady())
	{
		m_geometry = std::move(geometry);
		m_verts.updateGeometry(m_geometry);
		return true;
	}
	return false;
}

bool Mesh::setInstances(InstanceBuffer instances)
{
	if (isReady() && m_verts.isReady())
	{
		m_instances = std::move(instances);
		m_verts.setInstances(m_instances);
		return true;
	}
	return false;
}

void Mesh::draw(Shader const& shader) const
{
	if (isReady() && m_verts.isReady() && shader.isReady())
	{
		shader.setS32(env::g_config.uniforms.transform.isUI, false);
		m_verts.draw(shader);
	}
	return;
}

void Mesh::render(Shader const& shader, Material const* pMaterial /* = nullptr */) const
{
	if (!pMaterial)
	{
		pMaterial = &m_material;
	}
	if (m_verts.isReady() && shader.isReady())
	{
		shader.setMaterial(*pMaterial);
		shader.bind(m_textures);
		shader.setS32(env::g_config.uniforms.transform.isUI, false);
		m_verts.draw(shader);
	}
	return;
}

DrawType Mesh::drawType() const
{
	return m_drawType;
}

VertexArray const& Mesh::verts() const
{
	return m_verts;
}

Geometry const& Mesh::geometry() const
{
	return m_geometry;
}

InstanceBuffer const& Mesh::instances() const
{
	return m_instances;
}
} // namespace le::gfx
