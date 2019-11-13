#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "le3d/core/log.hpp"

namespace le
{
#define glChk() le::glCheckError(__FILE__, __LINE__)

inline GLenum glCheckError(const char* file, s32 line)
{
	GLenum errorCode = 0;
	errorCode = glGetError();
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string_view error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}
		LOG_E("[GLError] %s | %s (%d)", error.data(), file, line);
	}
	return errorCode;
}
} // namespace le
