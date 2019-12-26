#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/context/context.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/core/log.hpp"

namespace le
{
s32 gfx::glCheckError(const char* szFile, s32 line)
{
	GLenum errorCode = 0;
	errorCode = glGetError();

	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string_view error = "Unknown error";
		std::string_view description = "No description";

		// Decode the error code
		switch (errorCode)
		{
		case GL_INVALID_ENUM:
		{
			error = "GL_INVALID_ENUM";
			description = "An unacceptable value has been specified for an enumerated argument.";
			break;
		}

		case GL_INVALID_VALUE:
		{
			error = "GL_INVALID_VALUE";
			description = "A numeric argument is out of range.";
			break;
		}

		case GL_INVALID_OPERATION:
		{
			error = "GL_INVALID_OPERATION";
			description = "The specified operation is not allowed in the current state.";
			break;
		}

		case GL_OUT_OF_MEMORY:
		{
			error = "GL_OUT_OF_MEMORY";
			description = "There is not enough memory left to execute the command.";
			break;
		}

			/*case GL_STACK_OVERFLOW:
			{
				error = "GL_STACK_OVERFLOW";
				description = "This command would cause a stack overflow.";
				break;
			}

			case GL_STACK_UNDERFLOW:
			{
				error = "GL_STACK_UNDERFLOW";
				description = "This command would cause a stack underflow.";
				break;
			}

			case GLEXT_GL_INVALID_FRAMEBUFFER_OPERATION:
			{
				error = "GL_INVALID_FRAMEBUFFER_OPERATION";
				description = "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".";
				break;
			}*/
		}
		LOG_E("[GLError] %s | %s | %s (%d)", error.data(), description.data(), szFile, line);
	}
	return (s32)errorCode;
}

void gfx::cropViewport(f32 spaceAR)
{
	const glm::vec2 vpSize = context::size();
	const f32 vpAR = vpSize.x / vpSize.y;
	spaceAR = spaceAR <= 0.0f ? vpAR : spaceAR;
	const f32 uiW = vpAR > spaceAR ? vpSize.x * spaceAR / vpAR : vpSize.x;
	const f32 uiH = vpAR < spaceAR ? vpSize.y * vpAR / spaceAR : vpSize.y;
	glViewport((s32)((vpSize.x - uiW) * 0.5f), (s32)((vpSize.y - uiH) * 0.5f), (s32)uiW, (s32)uiH);
}

void gfx::resetViewport()
{
	const glm::vec2 vpSize = context::size();
	glViewport(0, 0, (s32)vpSize.x, (s32)vpSize.y);
}
} // namespace le