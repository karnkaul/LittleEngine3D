#include <array>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/context/context.hpp"
#include "context/contextImpl.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/gfxtypes.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
void HShader::use() const
{
	ASSERT(glID.handle > 0, "Invalid shader program!");
	glChk(glUseProgram(glID.handle));
}

bool HShader::setBool(std::string_view id, bool bVal) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID.handle, id.data());
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
		auto glID_ = glGetUniformLocation(glID.handle, id.data());
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
		auto glID_ = glGetUniformLocation(glID.handle, id.data());
		if (glID_ >= 0)
		{
			use();
			glChk(glUniform1f(glID_, val));
			return true;
		}
	}
	return false;
}

bool HShader::setV2(std::string_view id, const glm::vec2& val) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID.handle, id.data());
		if (glID_ >= 0)
		{
			use();
			glChk(glUniform2f(glID_, val.x, val.y));
			return true;
		}
	}
	return false;
}

bool HShader::setV3(std::string_view id, const glm::vec3& val) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID.handle, id.data());
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

bool HShader::setV4(std::string_view id, const glm::vec4& val) const
{
	if (!id.empty())
	{
		auto glID_ = glGetUniformLocation(glID.handle, id.data());
		if (glID_ >= 0)
		{
			use();
			glChk(glUniform4f(glID_, val.x, val.y, val.z, val.w));
			return true;
		}
	}
	return false;
}

void HShader::setModelMats(const ModelMats& mats) const
{
	use();
	auto temp = glGetUniformLocation(glID.handle, env::g_config.uniforms.modelMatrix.data());
	glChk(glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(mats.model)));
	auto pNMat = &mats.model;
	if (mats.oNormals)
	{
		pNMat = &(*mats.oNormals);
	}
	temp = glGetUniformLocation(glID.handle, env::g_config.uniforms.normalMatrix.data());
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(*pNMat));
}

void HShader::bindUBO(std::string_view id, const HUBO& ubo) const
{
	u32 idx = glGetUniformBlockIndex(glID.handle, id.data());
	if ((s32)idx >= 0)
	{
		glChk(glUniformBlockBinding(glID.handle, idx, ubo.bindingPoint));
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

void Vertices::addPoint(const glm::vec3& point)
{
	points.push_back({point.x, point.y, point.z});
}

void Vertices::addNormals(const glm::vec3& normal, u16 count)
{
	for (u16 i = 0; i < count; ++i)
	{
		normals.push_back({normal.x, normal.y, normal.z});
	}
}

void Vertices::addTexCoord(const glm::vec2& texCoord)
{
	texCoords.push_back({texCoord.x, texCoord.y});
}

u32 Vertices::addVertex(const glm::vec3& point, const glm::vec3& normal, std::optional<glm::vec2> oTexCoord)
{
	points.push_back({point.x, point.y, point.z});
	normals.push_back({normal.x, normal.y, normal.z});
	if (oTexCoord)
	{
		texCoords.push_back({oTexCoord->x, oTexCoord->y});
	}
	return (u32)points.size() - 1;
}

void Vertices::addIndices(const std::vector<u32> newIndices)
{
	std::copy(newIndices.begin(), newIndices.end(), std::back_inserter(indices));
}

bool operator==(const Vertices::V3& lhs, const Vertices::V3& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool operator==(const Vertices::V2& lhs, const Vertices::V2& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(const Vertices::V3& lhs, const Vertices::V3& rhs)
{
	return !(lhs == rhs);
}

bool operator!=(const Vertices::V2& lhs, const Vertices::V2& rhs)
{
	return !(lhs == rhs);
}
} // namespace le
