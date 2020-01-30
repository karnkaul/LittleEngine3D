#include <array>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/le3dgl.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/engine/gfx/gfxtypes.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "engine/contextImpl.hpp"

namespace le
{
void HShader::use() const
{
	ASSERT(glID > 0, "Invalid shader program!");
	glChk(glUseProgram(glID));
}

bool HShader::setBool(std::string_view id, bool bVal) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID, id.data());
		if (glID_ >= 0)
		{
			use();
			glChk(glUniform1i(glID_, static_cast<GLint>(bVal)));
			return true;
		}
	}
	return false;
}

bool HShader::setS32(std::string_view id, s32 val) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID, id.data());
		if (glID_ >= 0)
		{
			use();
			glChk(glUniform1i(glID_, static_cast<GLint>(val)));
			return true;
		}
	}
	return false;
}

bool HShader::setF32(std::string_view id, f32 val) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID, id.data());
		if (glID_ >= 0)
		{
			use();
			glChk(glUniform1f(glID_, val));
			return true;
		}
	}
	return false;
}

bool HShader::setV2(std::string_view id, glm::vec2 const& val) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID, id.data());
		if (glID_ >= 0)
		{
			use();
			glChk(glUniform2f(glID_, val.x, val.y));
			return true;
		}
	}
	return false;
}

bool HShader::setV3(std::string_view id, glm::vec3 const& val) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID, id.data());
		if (glID_ >= 0)
		{
			use();
			glChk(glUniform3f(glID_, val.x, val.y, val.z));
			return true;
		}
	}
	return false;
}

bool HShader::setV4(std::string_view id, Colour colour) const
{
	return setV4(id, glm::vec4(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
}

bool HShader::setV4(std::string_view id, glm::vec4 const& val) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID, id.data());
		if (glID_ >= 0)
		{
			use();
			glChk(glUniform4f(glID_, val.x, val.y, val.z, val.w));
			return true;
		}
	}
	return false;
}

void HShader::setModelMats(ModelMats const& mats) const
{
	use();
	auto temp = glGetUniformLocation(glID, env::g_config.uniforms.modelMatrix.data());
	glChk(glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(mats.model)));
	auto pNMat = &mats.model;
	if (mats.oNormals)
	{
		pNMat = &(*mats.oNormals);
	}
	temp = glGetUniformLocation(glID, env::g_config.uniforms.normalMatrix.data());
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(*pNMat));
}

void HShader::bindUBO(std::string_view id, HUBO const& ubo) const
{
	u32 idx = glGetUniformBlockIndex(glID, id.data());
	if ((s32)idx >= 0)
	{
		glChk(glUniformBlockBinding(glID, idx, ubo.bindingPoint));
	}
}

u32 Vertices::byteCount() const
{
	return (u32)((points.size() + normals.size()) * sizeof(V3) + texCoords.size() * sizeof(V2));
}

u32 Vertices::vertexCount() const
{
	return (u32)points.size();
}

void Vertices::addPoint(glm::vec3 const& point)
{
	points.push_back({point.x, point.y, point.z});
}

void Vertices::addNormals(glm::vec3 const& normal, u16 count)
{
	for (u16 i = 0; i < count; ++i)
	{
		normals.push_back({normal.x, normal.y, normal.z});
	}
}

void Vertices::addTexCoord(glm::vec2 const& texCoord)
{
	texCoords.push_back({texCoord.x, texCoord.y});
}

void Vertices::reserve(u32 vCount, u32 iCount)
{
	points.reserve(vCount);
	normals.reserve(vCount);
	texCoords.reserve(vCount);
	indices.reserve(iCount);
}

u32 Vertices::addVertex(glm::vec3 const& point, glm::vec3 const& normal, std::optional<glm::vec2> oTexCoord)
{
	points.push_back({point.x, point.y, point.z});
	normals.push_back({normal.x, normal.y, normal.z});
	if (oTexCoord)
	{
		texCoords.push_back({oTexCoord->x, oTexCoord->y});
	}
	return (u32)points.size() - 1;
}

void Vertices::addIndices(std::vector<u32> const& newIndices)
{
	std::copy(newIndices.begin(), newIndices.end(), std::back_inserter(indices));
}

bool operator==(Vertices::V3 const& lhs, Vertices::V3 const& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool operator==(Vertices::V2 const& lhs, Vertices::V2 const& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(Vertices::V3 const& lhs, Vertices::V3 const& rhs)
{
	return !(lhs == rhs);
}

bool operator!=(Vertices::V2 const& lhs, Vertices::V2 const& rhs)
{
	return !(lhs == rhs);
}
} // namespace le
