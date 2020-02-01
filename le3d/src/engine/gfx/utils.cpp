#include <deque>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfxtypes.hpp"
#include "le3d/engine/gfx/le3dgl.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/core/log.hpp"

namespace le
{
namespace
{
Rect2 g_view;
}

s32 gfx::glCheckError(char const* szFile, s32 line)
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

void gfx::setViewport(Rect2 const& view)
{
	auto bl = view.bl + context::windowSize() * 0.5f;
	auto size = view.size();
	setViewport((s32)bl.x, (s32)bl.y, (s32)size.x, (s32)size.y);
}

void gfx::setViewport(s32 x, s32 y, s32 dx, s32 dy)
{
	glViewport(x, y, dx, dy);
}

//Rect2 gfx::cropView(glm::vec2 const& viewSize, f32 spaceAR)
//{
//	f32 const vpAR = viewSize.x / viewSize.y;
//	spaceAR = spaceAR <= 0.0f ? vpAR : spaceAR;
//	f32 const uiW = vpAR > spaceAR ? viewSize.x * spaceAR / vpAR : viewSize.x;
//	f32 const uiH = vpAR < spaceAR ? viewSize.y * vpAR / spaceAR : viewSize.y;
//	return Rect2::sizeCentre({uiW, uiH});
//	// glViewport((s32)((viewSize.x - uiW) * 0.5f), (s32)((viewSize.y - uiH) * 0.5f), (s32)uiW, (s32)uiH);
//}

Rect2 gfx::cropView(Rect2 const& view, f32 spaceAspect)
{
	f32 const viewAspect = view.aspect();
	glm::vec2 const viewSize = view.size();
	if (spaceAspect <= 0.0f)
	{
		spaceAspect = viewAspect;
	}
	f32 const width = viewAspect > spaceAspect ? viewSize.x * spaceAspect / viewAspect : viewSize.x;
	f32 const height = viewAspect < spaceAspect ? viewSize.y * viewAspect / spaceAspect : viewSize.y;
	return Rect2::sizeCentre({width, height}, view.centre());
}

Rect2 const& gfx::view()
{
	if (glm::length2(g_view.size()) == 0.0f)
	{
		g_view = Rect2::sizeCentre(context::windowSize());
	}
	return g_view;
}

void gfx::setView(Rect2 const& view)
{
	g_view = view;
	setViewport(g_view);
}
} // namespace le
