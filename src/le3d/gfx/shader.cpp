#include <array>
#include <glad/glad.h>
#include "le3d/context/context.hpp"
#include "le3d/context/contextImpl.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
const u8 Shader::MAX_DIR_LIGHTS = 4;
const u8 Shader::MAX_POINT_LIGHTS = 4;

Shader::Shader() = default;
Shader::~Shader()
{
	if (m_bInit && m_program && context::exists())
	{
		glChk(glDeleteProgram(m_program));
		LOG_I("-- [%s] %s destroyed", m_id.data(), m_type.data());
	}
}
Shader::Shader(Shader&&) = default;
Shader& Shader::operator=(Shader&&) = default;

bool Shader::glSetup(std::string id, std::string_view vertCode, std::string_view fragCode)
{
	m_type = Typename(*this);
	s32 success;
	m_id = std::move(id);

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
		LOG_E("[%s] (%s) Failed to compile vertex shader!\n\t%s", m_id.data(), m_type.data(), buf.data());
	}

	u32 fsh = glCreateShader(GL_FRAGMENT_SHADER);
	files[1] = fragCode.data();
	glShaderSource(fsh, (GLsizei)ARR_SIZE(files), files, nullptr);
	glCompileShader(fsh);
	glGetShaderiv(fsh, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fsh, (GLsizei)buf.size(), nullptr, buf.data());
		LOG_E("[%s] (%s) Failed to compile fragment shader!\n\t%s", m_id.data(), m_type.data(), buf.data());
	}

	m_program = glCreateProgram();
	glAttachShader(m_program, vsh);
	glAttachShader(m_program, fsh);
	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(m_program, (GLsizei)buf.size(), nullptr, buf.data());
		LOG_E("[%s] (%s) Failed to link shaders!\n\t%s", m_id.data(), m_type.data(), buf.data());
		glDeleteProgram(m_program);
		m_program = 0;
	}

	glDeleteShader(vsh);
	glDeleteShader(fsh);
	m_bInit = success != 0;
	LOGIF_I(m_bInit, "== [%s] (%s) created", m_id.data(), m_type.data());
	return m_bInit;
}

void Shader::use() const
{
	glChk(glUseProgram(m_program));
}

bool Shader::setBool(std::string_view id, bool bVal) const
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(m_program, id.data());
		if (glID >= 0)
		{
			use();
			glChk(glUniform1i(glID, static_cast<GLint>(bVal)));
			return true;
		}
	}
	return false;
}

bool Shader::setS32(std::string_view id, s32 val) const
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(m_program, id.data());
		if (glID >= 0)
		{
			use();
			glChk(glUniform1i(glID, static_cast<GLint>(val)));
			return true;
		}
	}
	return false;
}

bool Shader::setF32(std::string_view id, f32 val) const
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(m_program, id.data());
		if (glID >= 0)
		{
			use();
			glChk(glUniform1f(glID, val));
			return true;
		}
	}
	return false;
}

bool Shader::setV2(std::string_view id, const glm::vec2& val) const
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(m_program, id.data());
		if (glID >= 0)
		{
			use();
			glChk(glUniform2f(glID, val.x, val.y));
			return true;
		}
	}
	return false;
}

bool Shader::setV3(std::string_view id, const glm::vec3& val) const
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(m_program, id.data());
		if (glID >= 0)
		{
			use();
			glChk(glUniform3f(glID, val.x, val.y, val.z));
			return true;
		}
	}
	return false;
}

bool Shader::setV4(std::string_view id, Colour colour) const
{
	return setV4(id, glm::vec4(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
}

bool Shader::setV4(std::string_view id, const glm::vec4& val) const
{
	if (!id.empty())
	{
		auto glID = glGetUniformLocation(m_program, id.data());
		if (glID >= 0)
		{
			use();
			glChk(glUniform4f(glID, val.x, val.y, val.z, val.w));
			return true;
		}
	}
	return false;
}

void Shader::setupLights(const std::vector<DirLight>& dirLights, const std::vector<PointLight>& pointLights) const
{
	use();
	size_t i;
	Light blank;
	blank.ambient = blank.diffuse = blank.specular = glm::vec3(0.0f);
	PointLight blankP;
	blankP.light = blank;
	DirLight blankD;
	blankD.light = blank;
	for (i = 0; i < MAX_DIR_LIGHTS; ++i)
	{
		const auto& dirLight = i < dirLights.size() ? dirLights[i] : blankD;
		std::string id = "dirLights[";
		id += std::to_string(i);
		id += "].";
		setV3(id + "ambient", dirLight.light.ambient);
		setV3(id + "diffuse", dirLight.light.diffuse);
		setV3(id + "specular", dirLight.light.specular);
		setV3(id + "direction", dirLight.direction);
	}
	for (i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		const auto& pointLight = i < pointLights.size() ? pointLights[i] : blankP;
		std::string id = "pointLights[";
		id += std::to_string(i);
		id += "].";
		setV3(id + "ambient", pointLight.light.ambient);
		setV3(id + "diffuse", pointLight.light.diffuse);
		setV3(id + "specular", pointLight.light.specular);
		setF32(id + "constant", pointLight.constant);
		setF32(id + "linear", pointLight.linear);
		setF32(id + "quadratic", pointLight.quadratic);
	}
}
} // namespace le
