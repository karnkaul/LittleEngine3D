#include <array>
#include <glad/glad.h>
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

void shading::use(Shader shader)
{
	ASSERT(shader.glID.handle > 0, "Invalid shader program!");
	glChk(glUseProgram(shader.glID.handle));
}

bool shading::setBool(Shader shader, std::string_view id, bool bVal)
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

bool shading::setS32(Shader shader, std::string_view id, s32 val)
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

bool shading::setF32(Shader shader, std::string_view id, f32 val)
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

bool shading::setV2(Shader shader, std::string_view id, const glm::vec2& val)
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

bool shading::setV3(Shader shader, std::string_view id, const glm::vec3& val)
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

bool shading::setV4(Shader shader, std::string_view id, Colour colour)
{
	return setV4(shader, id, glm::vec4(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
}

bool shading::setV4(Shader shader, std::string_view id, const glm::vec4& val)
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

void shading::setupLights(Shader shader, const std::vector<DirLight>& dirLights, const std::vector<PtLight>& pointLights)
{
	use(shader);
	size_t i;
	LightData blank;
	blank.ambient = blank.diffuse = blank.specular = glm::vec3(0.0f);
	PtLight blankP;
	blankP.light = blank;
	DirLight blankD;
	blankD.light = blank;
	for (i = 0; i < MAX_DIR_LIGHTS; ++i)
	{
		const auto& dirLight = i < dirLights.size() ? dirLights[i] : blankD;
		std::string id = "dirLights[";
		id += std::to_string(i);
		id += "].";
		setV3(shader, id + "ambient", dirLight.light.ambient);
		setV3(shader, id + "diffuse", dirLight.light.diffuse);
		setV3(shader, id + "specular", dirLight.light.specular);
		setV3(shader, id + "direction", dirLight.direction);
	}
	for (i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		const auto& pointLight = i < pointLights.size() ? pointLights[i] : blankP;
		std::string id = "pointLights[";
		id += std::to_string(i);
		id += "].";
		setV3(shader, id + "ambient", pointLight.light.ambient);
		setV3(shader, id + "diffuse", pointLight.light.diffuse);
		setV3(shader, id + "specular", pointLight.light.specular);
		setF32(shader, id + "constant", pointLight.constant);
		setF32(shader, id + "linear", pointLight.linear);
		setF32(shader, id + "quadratic", pointLight.quadratic);
		setV3(shader, id + "position", pointLight.position);
	}
}
} // namespace le::gfx
