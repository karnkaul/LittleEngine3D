#include <glm/gtx/quaternion.hpp>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/maths.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/engine/input.hpp"
#include "le3d/game/ecs/components/ccamera.hpp"

namespace le
{
bool s_bTEST = false;
glm::mat4 CCamera::view() const
{
#if defined(DEBUGGING)
	if (s_bTEST)
	{
		glm::vec3 const nFront = glm::normalize(glm::rotate(m_orientation, g_nFront));
		glm::vec3 const nUp = glm::normalize(glm::rotate(m_orientation, g_nUp));
		return glm::lookAt(m_position, m_position - nFront, nUp);
	}
	else
#endif
	{
		return glm::toMat4(glm::conjugate(m_orientation)) * glm::translate(glm::mat4(1.0f), -m_position);
	}
}

glm::mat4 CCamera::perspectiveProj(f32 near, f32 far) const
{
	return glm::perspective(glm::radians(m_fov), m_aspectRatio > 0.0f ? m_aspectRatio : context::windowAspect(), near, far);
}

glm::mat4 CCamera::orthographicProj(f32 zoom, f32 near, f32 far) const
{
	ASSERT(zoom > 0.0f, "Invalid zoom!");
	f32 ar = m_aspectRatio > 0.0f ? m_aspectRatio : context::windowAspect();
	f32 w = ar > 1.0f ? 1.0f : 1.0f * ar;
	f32 h = ar > 1.0f ? 1.0f / ar : 1.0f;
	return glm::ortho(-w / zoom, w / zoom, -h / zoom, h / zoom, near, far);
}

glm::mat4 CCamera::uiProj(glm::vec3 const& uiSpace) const
{
	f32 const w = uiSpace.x * 0.5f;
	f32 const h = uiSpace.y * 0.5f;
	return glm::ortho(-w, w, -h, h, -uiSpace.z, uiSpace.z);
}

void CFreeCam::onCreate()
{
	m_tMove = input::registerInput([this](Key key, Action action, Mods /*mods*/) {
		if (m_flags.isSet(Flag::Enabled))
		{
			switch (action)
			{
			case Action::PRESS:
			{
				m_heldKeys.emplace(key);
				break;
			}
			case Action::RELEASE:
			{
				if (!m_flags.isSet(Flag::FixedSpeed) && key == Key::MOUSE_BUTTON_3)
				{
					m_speed = m_defaultSpeed;
				}
				m_heldKeys.erase(key);
				break;
			}
			default:
				break;
			}
			if (key == Key::MOUSE_BUTTON_2)
			{
				bool bLook = action == Action::PRESS;
				if (m_flags.isSet(Flag::Looking) ^ bLook)
				{
					m_flags.set(Flag::InitPos, false);
				}
				m_flags.set(Flag::Looking, bLook);
				input::setCursorMode(bLook ? CursorMode::Disabled : CursorMode::Default);
			}
		}
	});
	m_tLook = input::registerMouse([this](f64 x, f64 y) {
		if (m_flags.isSet(Flag::Enabled) && m_flags.isSet(Flag::Looking))
		{
			m_nextCursorPos = {(f32)x, (f32)y};
			if (!m_flags.isSet(Flag::InitPos))
			{
				m_cursorPos = {(f32)x, (f32)y};
				m_flags.set(Flag::InitPos, true);
			}
		}
	});
	m_tZoom = input::registerScroll([this](f64 /*dx*/, f64 dy) { m_dSpeed += (f32)dy; });
	m_tFocus = input::registerFocus([this](bool /*bFocus*/) { m_flags.set(Flag::InitPos, false); });
	m_flags.set(Flag::Enabled, true);
	return;
}
} // namespace le
