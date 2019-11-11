#include <array>
#include <glad/glad.h>
#include "le3d/log/log.hpp"
#include "le3d/gfx/shader.hpp"

namespace le
{
Shader::Shader() = default;
Shader::~Shader() 
{
	if (m_bInit && m_program)
	{
		glDeleteProgram(m_program);
		logI("-- [%s] %s destroyed", m_id.data(), m_type.data());
	}
}
Shader::Shader(Shader&&) = default;
Shader& Shader::operator=(Shader&&) = default;

bool Shader::init(std::string id, std::string_view vertCode, std::string_view fragCode)
{
	m_type = Typename(*this);
	std::array<const GLchar*, 1> files;
	s32 success;
	m_id = std::move(id);

	u32 vsh = glCreateShader(GL_VERTEX_SHADER);
	files = {vertCode.data()};
	glShaderSource(vsh, files.size(), files.data(), nullptr);
	glCompileShader(vsh);
	std::array<char, 512> buf;
	glGetShaderiv(vsh, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vsh, buf.size(), nullptr, buf.data());
		logE("[%s] (%s) Failed to compile vertex shader!\n\t%s", m_id.data(), m_type.data(), buf.data());
	}
	
	u32 fsh = glCreateShader(GL_FRAGMENT_SHADER);
	files = {fragCode.data()};
	glShaderSource(fsh, files.size(), files.data(), nullptr);
	glCompileShader(fsh);
	glGetShaderiv(fsh, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vsh, buf.size(), nullptr, buf.data());
		logE("[%s] (%s) Failed to compile fragment shader!\n\t%s", m_id.data(), m_type.data(), buf.data());
	}
	
	m_program = glCreateProgram();
	glAttachShader(m_program, vsh);
	glAttachShader(m_program, fsh);
	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(m_program, buf.size(), nullptr, buf.data());
		logE("[%s] (%s) Failed to link shaders!\n\t%s", m_id.data(), m_type.data(), buf.data());
		glDeleteProgram(m_program);
		m_program = 0;
	}

	glDeleteShader(vsh);
	glDeleteShader(fsh);
	m_bInit = success != 0;
	logifI(m_bInit, "== [%s] (%s) created", m_id.data(), m_type.data());
	return m_bInit;
}

u32 Shader::program() const
{
	return m_program;
}

void Shader::setupAttribs()
{
	GLint position = glGetAttribLocation(m_program, "position");
	if (position >= 0)
	{
		auto glPos = toGLObj(position);
		glVertexAttribPointer(glPos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(glPos);
	}
}
} // namespace le
