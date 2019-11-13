#include <assert.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "le3d/core/log.hpp"
#include "le3d/core/maths.hpp"
#include "le3d/context/context.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
Camera::Camera() = default;
Camera::Camera(Camera&&) = default;
Camera& Camera::operator=(Camera&&) = default;
Camera::~Camera() = default;

void Camera::tick(Time /*dt*/) {}

bool s_bTEST = false;
glm::mat4 Camera::view() const
{
#if defined(DEBUGGING)
	if (s_bTEST)
	{
		const glm::vec3 nFront = glm::normalize(glm::rotate(m_orientation, g_nFront));
		const glm::vec3 nUp = glm::normalize(glm::rotate(m_orientation, g_nUp));
		return glm::lookAt(m_position, m_position - nFront, nUp);
	}
	else
#endif
	{
		return glm::toMat4(glm::conjugate(m_orientation)) * glm::translate(glm::mat4(1.0f), -m_position);
	}
}

glm::mat4 Camera::perspectiveProj(f32 aspect, f32 near, f32 far) const
{
	return glm::perspective(glm::radians(m_fov), aspect, near, far);
}

glm::mat4 Camera::orthographicProj(glm::vec4 lrbt, f32 near, f32 far) const
{
	return glm::ortho(lrbt.x, lrbt.y, lrbt.z, lrbt.w, near, far);
}

FreeCam::FreeCam()
{
	m_tMove = input::registerInput([this](s32 key, s32 action, s32 /*mods*/) {
		if (m_flags.isSet((s32)Flag::Enabled))
		{
			switch (action)
			{
			case GLFW_PRESS:
			{
				m_heldKeys.emplace(key);
				break;
			}
			case GLFW_RELEASE:
			{
				if (!m_flags.isSet((s32)Flag::FixedSpeed) && key == GLFW_MOUSE_BUTTON_3)
				{
					m_speed = m_defaultSpeed;
				}
				m_heldKeys.erase(key);
				break;
			}
			default:
				break;
			}
			if (key == GLFW_MOUSE_BUTTON_2)
			{
				bool bLook = action == GLFW_PRESS;
				if (m_flags.isSet((s32)Flag::Looking) ^ bLook)
				{
					m_flags.set((s32)Flag::InitPos, false);
				}
				m_flags.set((s32)Flag::Looking, bLook);
				input::setCursorMode(bLook ? CursorMode::Disabled : CursorMode::Default);
			}
		}
	});
	m_tLook = input::registerMouse([this](f64 x, f64 y) {
		if (m_flags.isSet((s32)Flag::Enabled) && m_flags.isSet((s32)Flag::Looking))
		{
			m_nextMousePos = {(f32)x, (f32)y};
			if (!m_flags.isSet((s32)Flag::InitPos))
			{
				m_mousePos = {(f32)x, (f32)y};
				m_flags.set((s32)Flag::InitPos, true);
			}
		}
	});
	m_tZoom = input::registerScroll([this](f32 /*dx*/, f32 dy) { m_dSpeed += dy; });
	m_tFocus = input::registerFocus([this](bool /*bFocus*/) { m_flags.set((s32)Flag::InitPos, false); });
	m_flags.set((s32)Flag::Enabled, true);
}

void FreeCam::tick(Time dt)
{
	if (!m_flags.isSet((s32)Flag::Enabled))
	{
		return;
	}

	// Speed
	if (!m_flags.isSet((s32)Flag::FixedSpeed) && m_dSpeed != 0.0f)
	{
		m_speed = Maths::clamp(m_speed + (m_dSpeed * dt.assecs() * 100), m_minSpeed, m_maxSpeed);
		m_dSpeed = Maths::lerp(m_dSpeed, 0.0f, 0.75f);
	}

	// Look
	if (m_mousePos != m_nextMousePos)
	{
		glm::vec2 delta = m_nextMousePos - m_mousePos;
		m_yaw = glm::normalize(glm::rotate(m_yaw, -delta.x * m_lookSens * dt.assecs(), g_nUp));
		m_pitch = glm::normalize(glm::rotate(m_pitch, -delta.y * m_lookSens * dt.assecs(), g_nRight));
		/*m_yaw += (delta.x * m_lookSens * dt.assecs());
		m_pitch += (delta.y* m_lookSens * dt.assecs());*/
		m_mousePos = m_nextMousePos;
	}
	m_orientation = m_yaw * m_pitch;
	/*m_orientation = glm::angleAxis(glm::radians(-m_pitch), g_nRight);
	m_orientation *= glm::angleAxis(glm::radians(-m_yaw), g_nUp);*/

	// Move
	m_dPos = glm::vec3(0.0f);
	const glm::vec3 nForward = -glm::normalize(glm::rotate(m_orientation, g_nFront));
	const glm::vec3 nRight = glm::normalize(glm::rotate(m_orientation, g_nRight));

	for (auto key : m_heldKeys)
	{
		switch (key)
		{
		default:
			break;

		case GLFW_KEY_W:
		case GLFW_KEY_UP:
		{
			m_dPos += nForward;
			break;
		}

		case GLFW_KEY_D:
		case GLFW_KEY_RIGHT:
		{
			m_dPos += nRight;
			break;
		}

		case GLFW_KEY_S:
		case GLFW_KEY_DOWN:
		{
			m_dPos -= nForward;
			break;
		}

		case GLFW_KEY_A:
		case GLFW_KEY_LEFT:
		{
			m_dPos -= nRight;
			break;
		}
		}
	}
	if (glm::length2(m_dPos) > 0.0f)
	{
		m_dPos = glm::normalize(m_dPos);
		m_position += (m_dPos * dt.assecs() * m_speed);
	}
}
} // namespace le
