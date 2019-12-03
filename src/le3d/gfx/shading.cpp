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
const u8 shading::MAX_PT_LIGHTS = 4;

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

void shading::setupLights(const HShader& shader, const std::vector<DirLight>& dirLights, const std::vector<PtLight>& ptLights)
{
	use(shader);
	auto temp = glGetUniformLocation(shader.glID.handle, "dirLightCount");
	glChk(glUniform1i(temp, (GLint)dirLights.size()));
	temp = glGetUniformLocation(shader.glID.handle, "ptLightCount");
	glChk(glUniform1i(temp, (GLint)ptLights.size()));

	size_t i;
	char buf[64];
	for (i = 0; i < dirLights.size() && i < MAX_DIR_LIGHTS; ++i)
	{
		const auto& dirLight = dirLights[i];
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
	for (i = 0; i < ptLights.size() && i < MAX_PT_LIGHTS; ++i)
	{
		const auto& ptLight = ptLights[i];
		{
			std::snprintf(buf, ARR_SIZE(buf), "ptLights[%d].ambient", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = ptLight.light.ambient;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "ptLights[%d].diffuse", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = ptLight.light.diffuse;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "ptLights[%d].specular", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = ptLight.light.specular;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "ptLights[%d].position", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			const auto& v = ptLight.position;
			glChk(glUniform3f(temp, v.x, v.y, v.z));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "ptLights[%d].constant", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			glChk(glUniform1f(temp, ptLight.constant));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "ptLights[%d].linear", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			glChk(glUniform1f(temp, ptLight.linear));
		}
		{
			std::snprintf(buf, ARR_SIZE(buf), "ptLights[%d].quadratic", (s32)i);
			auto temp = glGetUniformLocation(shader.glID.handle, buf);
			glChk(glUniform1f(temp, ptLight.quadratic));
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
	if (ubo.ubo.handle > 0)
	{
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, ubo.ubo.handle));
		glChk(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, pData));
		glChk(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}
}
} // namespace le::gfx
