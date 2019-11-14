#include <array>
#include <glad/glad.h>
#include "le3d/context/context.hpp"
#include "le3d/context/contextImpl.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
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
	std::array<const GLchar*, 1> files;
	s32 success;
	m_id = std::move(id);

	Lock lock(context::g_glMutex);
	u32 vsh = glCreateShader(GL_VERTEX_SHADER);
	files = {vertCode.data()};
	glShaderSource(vsh, (GLsizei)files.size(), files.data(), nullptr);
	glCompileShader(vsh);
	std::array<char, 512> buf;
	glGetShaderiv(vsh, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vsh, (GLsizei)buf.size(), nullptr, buf.data());
		LOG_E("[%s] (%s) Failed to compile vertex shader!\n\t%s", m_id.data(), m_type.data(), buf.data());
	}

	u32 fsh = glCreateShader(GL_FRAGMENT_SHADER);
	files = {fragCode.data()};
	glShaderSource(fsh, (GLsizei)files.size(), files.data(), nullptr);
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
} // namespace le
