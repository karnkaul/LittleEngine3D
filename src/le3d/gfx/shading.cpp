#include <array>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/context/context.hpp"
#include "le3d/context/contextImpl.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/gfx/utils.hpp"

namespace le::gfx
{
const u8 shading::MAX_DIR_LIGHTS = 4;
const u8 shading::MAX_POINT_LIGHTS = 4;

void shading::use(const HShader& shader)
{
	ASSERT(shader.glID.handle > 0, "Invalid shader program!");
	glChk(glUseProgram(shader.glID.handle));
}

bool shading::setBool(const HShader& shader, std::string_view id, bool bVal)
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(shader.glID.handle, id.data());
		if (glID >= 0)
		{
			use(shader);
			glChk(glUniform1i(glID, static_cast<GLint>(bVal)));
			return true;
		}
	}
	return false;
}

bool shading::setS32(const HShader& shader, std::string_view id, s32 val)
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(shader.glID.handle, id.data());
		if (glID >= 0)
		{
			use(shader);
			glChk(glUniform1i(glID, static_cast<GLint>(val)));
			return true;
		}
	}
	return false;
}

bool shading::setF32(const HShader& shader, std::string_view id, f32 val)
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(shader.glID.handle, id.data());
		if (glID >= 0)
		{
			use(shader);
			glChk(glUniform1f(glID, val));
			return true;
		}
	}
	return false;
}

bool shading::setV2(const HShader& shader, std::string_view id, const glm::vec2& val)
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(shader.glID.handle, id.data());
		if (glID >= 0)
		{
			use(shader);
			glChk(glUniform2f(glID, val.x, val.y));
			return true;
		}
	}
	return false;
}

bool shading::setV3(const HShader& shader, std::string_view id, const glm::vec3& val)
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(shader.glID.handle, id.data());
		if (glID >= 0)
		{
			use(shader);
			glChk(glUniform3f(glID, val.x, val.y, val.z));
			return true;
		}
	}
	return false;
}

bool shading::setV4(const HShader& shader, std::string_view id, Colour colour)
{
	return setV4(shader, id, glm::vec4(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
}

bool shading::setV4(const HShader& shader, std::string_view id, const glm::vec4& val)
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(shader.glID.handle, id.data());
		if (glID >= 0)
		{
			use(shader);
			glChk(glUniform4f(glID, val.x, val.y, val.z, val.w));
			return true;
		}
	}
	return false;
}

void shading::setModelMats(const HShader& shader, const glm::mat4& model, const glm::mat4& normals)
{
	use(shader);
	auto temp = glGetUniformLocation(shader.glID.handle, "model");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(model));
	temp = glGetUniformLocation(shader.glID.handle, "normalModel");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(normals));
}

void shading::setViewMats(const HShader& shader, const glm::mat4& view, const glm::mat4& proj)
{
	use(shader);
	auto temp = glGetUniformLocation(shader.glID.handle, "view");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(view));
	temp = glGetUniformLocation(shader.glID.handle, "projection");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(proj));
	temp = glGetUniformLocation(shader.glID.handle, "viewPos");
	glChk(glUniform3f(temp, -view[3][0], -view[3][1], -view[3][2]));
}

void shading::setProjMat(const HShader& shader, const glm::mat4& proj)
{
	use(shader);
	auto temp = glGetUniformLocation(shader.glID.handle, "projection");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(proj));
}

void shading::setAllMats(const HShader& shader, const glm::mat4& m, const glm::mat4& n, const glm::mat4& v, const glm::mat4& p)
{
	use(shader);
	auto temp = glGetUniformLocation(shader.glID.handle, "model");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(m));
	temp = glGetUniformLocation(shader.glID.handle, "normalModel");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(n));
	temp = glGetUniformLocation(shader.glID.handle, "view");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(v));
	temp = glGetUniformLocation(shader.glID.handle, "projection");
	glUniformMatrix4fv(temp, 1, GL_FALSE, glm::value_ptr(p));
	temp = glGetUniformLocation(shader.glID.handle, "viewPos");
	glChk(glUniform3f(temp, -v[3][0], -v[3][1], -v[3][2]));
}

void shading::setupLights(const HShader& shader, const std::vector<DirLight>& dirLights, const std::vector<PtLight>& pointLights)
{
	use(shader);
	size_t i;
	LightData blank;
	blank.ambient = blank.diffuse = blank.specular = glm::vec3(0.0f);
	PtLight blankP;
	blankP.light = blank;
	DirLight blankD;
	blankD.light = blank;
	char buf[64];
	for (i = 0; i < MAX_DIR_LIGHTS; ++i)
	{
		const auto& dirLight = i < dirLights.size() ? dirLights[i] : blankD;
		{
			std::snprintf(buf, ARR_SIZE(buf), "dirLights[%d].ambient", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = dirLight.light.ambient;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "dirLights[%d].diffuse", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = dirLight.light.diffuse;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "dirLights[%d].specular", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = dirLight.light.specular;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "dirLights[%d].direction", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = dirLight.direction;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
	}
	for (i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		const auto& pointLight = i < pointLights.size() ? pointLights[i] : blankP;
		{
			std::snprintf(buf, ARR_SIZE(buf), "pointLights[%d].ambient", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = pointLight.light.ambient;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "pointLights[%d].diffuse", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = pointLight.light.diffuse;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "pointLights[%d].specular", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = pointLight.light.specular;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "pointLights[%d].position", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = pointLight.position;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "pointLights[%d].constant", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			glChk(glUniform1f(temp, pointLight.constant));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "pointLights[%d].linear", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			glChk(glUniform1f(temp, pointLight.linear));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "pointLights[%d].quadratic", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			glChk(glUniform1f(temp, pointLight.quadratic));
		}
	}
}

void shading::bindUBO(const HShader& shader, std::string_view id, const HUBO& ubo)
{
	u32 idx = glGetUniformBlockIndex(shader.glID.handle, id.data());
	if ((s32)idx >= 0)
	{
		glChk(glUniformBlockBinding(shader.glID.handle, idx, ubo.bindingPoint));
	}
}

void shading::setUBO(const HUBO& ubo, s64 offset, s64 size, const void* pData)
{
	glChk(glBindBuffer(GL_UNIFORM_BUFFER, ubo.ubo.handle));
	glChk(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, pData));
	glChk(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}
} // namespace le::gfx
