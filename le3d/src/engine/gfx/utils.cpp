#include <deque>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/core/log.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfx_objects.hpp"
#include "le3d/engine/gfx/gfx_thread.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "engine/gfx/le3dgl.hpp"

namespace le
{
namespace
{
Rect2 g_view;

GLenum cast(StringProp prop)
{
	switch (prop)
	{
	default:
		return 0;
	case StringProp::Renderer:
		return GL_RENDERER;
	case StringProp::Vendor:
		return GL_VENDOR;
	case StringProp::Version:
		return GL_VERSION;
	}
}
} // namespace

bool gfx::loadFunctionPointers(GLLoadProc loadFunc)
{
#if defined(LE3D_USE_GLAD)
	if (!gladLoadGLLoader((GLADloadproc)loadFunc))
	{
		return false;
	}
#endif
	return true;
}

void gfx::clearFlags(ClearFlags flags, Colour colour)
{
	if (context::isAlive())
	{
		enqueue([flags, colour]() {
			GLbitfield glFlags = 0;
			if (flags.isSet(ClearFlag::ColorBuffer))
			{
				glChk(glClearColor(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
				glFlags |= GL_COLOR_BUFFER_BIT;
			}
			if (flags.isSet(ClearFlag::DepthBuffer))
			{
				glFlags |= GL_DEPTH_BUFFER_BIT;
			}
			if (flags.isSet(ClearFlag::StencilBuffer))
			{
				glFlags |= GL_STENCIL_BUFFER_BIT;
			}
			glChk(glClear(glFlags));
		});
	}
	return;
}

void gfx::setFlag(GLFlag flag, bool bEnable)
{
	GLenum glFlag = 0;
	switch (flag)
	{
	default:
		break;
	case GLFlag::DepthTest:
		glFlag = GL_DEPTH_TEST;
		break;
	case GLFlag::Blend:
		glFlag = GL_BLEND;
		break;
	}
	if (glFlag > 0)
	{
		if (bEnable)
		{
			gfx::enqueue([glFlag]() { glChk(glEnable(glFlag)); });
		}
		else
		{
			gfx::enqueue([glFlag]() { glChk(glDisable(glFlag)); });
		}
	}
	return;
}

void gfx::setBlendFunc(BlendFunc func)
{
	GLenum sFactor = 0, dFactor = 0;
	switch (func)
	{
	default:
		break;
	case BlendFunc::Src_OneMinusSrc:
		sFactor = GL_SRC_ALPHA;
		dFactor = GL_ONE_MINUS_SRC_ALPHA;
		break;
	}
	if (sFactor > 0 && dFactor > 0)
	{
		gfx::enqueue([sFactor, dFactor]() { glChk(glBlendFunc(sFactor, dFactor)); });
	}
}

void gfx::setPolygonMode(PolygonMode mode, PolygonFace face)
{
	GLenum glFace, glMode;
	switch (face)
	{
	default:
	case PolygonFace::FrontAndBack:
		glFace = GL_FRONT_AND_BACK;
		break;
	case PolygonFace::Front:
		glFace = GL_FRONT;
		break;
	case PolygonFace::Back:
		glFace = GL_BACK;
		break;
	}
	switch (mode)
	{
	default:
	case PolygonMode::Fill:
		glMode = GL_FILL;
		break;
	case PolygonMode::Line:
		glMode = GL_LINE;
		break;
	}
	gfx::enqueue([glFace, glMode]() { glChk(glPolygonMode(glFace, glMode)); });
	return;
}

void gfx::glCheckError(char const* szFile, s32 line)
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
	return;
}

void gfx::setViewport(Rect2 const& view)
{
	auto bl = view.bl + context::windowSize() * 0.5f;
	auto size = view.size();
	setViewport((s32)bl.x, (s32)bl.y, (s32)size.x, (s32)size.y);
	return;
}

void gfx::setViewport(s32 x, s32 y, s32 dx, s32 dy)
{
	gfx::enqueue([x, y, dx, dy]() { glViewport(x, y, dx, dy); });
	return;
}

// Rect2 gfx::cropView(glm::vec2 const& viewSize, f32 spaceAR)
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
	return;
}

std::string_view gfx::getString(StringProp prop)
{
	GLenum glName = cast(prop);
	std::string_view ret;
	if (glName > 0)
	{
		ret = (char const*)glGetString(glName);
	}
	return ret;
}
} // namespace le
