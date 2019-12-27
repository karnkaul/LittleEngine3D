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

u32 Vertices::bytes() const
{
	u32 total = (u32)(points.size() * sizeof(f32));
	total += (u32)(normals.size() * sizeof(f32));
	total += (u32)(texCoords.size() * sizeof(f32));
	return total;
}

u32 Vertices::vertexCount() const
{
	return (u32)points.size() / 3;
}

void Vertices::addPoint(glm::vec3 point)
{
	points.push_back(point.x);
	points.push_back(point.y);
	points.push_back(point.z);
}

void Vertices::addNormals(glm::vec3 normal, u16 count)
{
	for (u16 i = 0; i < count; ++i)
	{
		normals.push_back(normal.x);
		normals.push_back(normal.y);
		normals.push_back(normal.z);
	}
}

void Vertices::addTexCoord(glm::vec2 texCoord)
{
	texCoords.push_back(texCoord.x);
	texCoords.push_back(texCoord.y);
}
} // namespace le
