#include <assert.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/maths.hpp"
#include "le3d/context/context.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
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

glm::mat4 Camera::perspectiveProj(f32 near, f32 far) const
{
	return glm::perspective(glm::radians(m_fov), context::nativeAR(), near, far);
}

glm::mat4 Camera::orthographicProj(f32 zoom, f32 near, f32 far) const
{
	ASSERT(zoom > 0.0f, "Invalid zoom!");
	f32 ar = context::nativeAR();
	f32 w = ar > 1.0f ? 1.0f : 1.0f * ar;
	f32 h = ar > 1.0f ? 1.0f / ar : 1.0f;
	return glm::ortho(-w / zoom, w / zoom, -h / zoom, h / zoom, near, far);
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
			m_nextCursorPos = {(f32)x, (f32)y};
			if (!m_flags.isSet((s32)Flag::InitPos))
			{
				m_cursorPos = {(f32)x, (f32)y};
				m_flags.set((s32)Flag::InitPos, true);
			}
		}
	});
	m_tZoom = input::registerScroll([this](f64 /*dx*/, f64 dy) { m_dSpeed += (f32)dy; });
	m_tFocus = input::registerFocus([this](bool /*bFocus*/) { m_flags.set((s32)Flag::InitPos, false); });
	m_flags.set((s32)Flag::Enabled, true);
}

void FreeCam::tick(Time dt)
{
	if (!m_flags.isSet((s32)Flag::Enabled))
	{
		return;
	}
	GamepadState pad0 = input::getGamepadState(0);

	// Speed
	if (!m_flags.isSet((s32)Flag::FixedSpeed))
	{
		if (pad0.isPressed(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER))
		{
			m_dSpeed -= (dt.assecs() * 10);
		}
		else if (pad0.isPressed(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER))
		{
			m_dSpeed += (dt.assecs() * 10);
		}
		if (m_dSpeed * m_dSpeed > 0.0f)
		{
			m_speed = Maths::clamp(m_speed + (m_dSpeed * dt.assecs() * 100), m_minSpeed, m_maxSpeed);
			m_dSpeed = Maths::lerp(m_dSpeed, 0.0f, 0.75f);
			if (m_dSpeed * m_dSpeed < 0.01f)
			{
				m_dSpeed = 0.0f;
			}
		}
	}

	// Elevation
	f32 elevation = input::triggerToAxis(pad0.getAxis(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER))
					- input::triggerToAxis(pad0.getAxis(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER));
	if (elevation * elevation > 0.01f)
	{
		m_position.y += (elevation * dt.assecs() * m_speed);
	}

	// Look
	f32 dLook = m_joyLookSens * dt.assecs();
	const glm::vec2 padRight(pad0.getAxis(GLFW_GAMEPAD_AXIS_RIGHT_X), pad0.getAxis(GLFW_GAMEPAD_AXIS_RIGHT_Y));
	if (glm::length2(padRight) > m_minJoyRightDPosSqr)
	{
		m_pitch += (padRight.y * dLook);
		m_yaw += (padRight.x * dLook);
	}

	dLook = m_mouseLookSens * dt.assecs();
	glm::vec2 dCursorPos = m_nextCursorPos - m_cursorPos;
	if (glm::length2(dCursorPos) > m_minCursorDPosSqr)
	{
		m_yaw += (dCursorPos.x * dLook);
		m_pitch += (dCursorPos.y * dLook);
		m_cursorPos = m_nextCursorPos;
	}
	glm::quat pitch = glm::angleAxis(glm::radians(-m_pitch), g_nRight);
	glm::quat yaw = glm::angleAxis(glm::radians(-m_yaw), g_nUp);
	m_orientation = yaw * pitch;

	// Move
	glm::vec3 dPos = glm::vec3(0.0f);
	const glm::vec3 nForward = -glm::normalize(glm::rotate(m_orientation, g_nFront));
	const glm::vec3 nRight = glm::normalize(glm::rotate(m_orientation, g_nRight));
	const glm::vec2 padLeft(pad0.getAxis(GLFW_GAMEPAD_AXIS_LEFT_X), -pad0.getAxis(GLFW_GAMEPAD_AXIS_LEFT_Y));

	if (glm::length2(padLeft) > m_minJoyRightDPosSqr)
	{
		dPos += (nForward * padLeft.y);
		dPos += (nRight * padLeft.x);
	}

	for (auto key : m_heldKeys)
	{
		switch (key)
		{
		default:
			break;

		case GLFW_KEY_W:
		case GLFW_KEY_UP:
		{
			dPos += nForward;
			break;
		}

		case GLFW_KEY_D:
		case GLFW_KEY_RIGHT:
		{
			dPos += nRight;
			break;
		}

		case GLFW_KEY_S:
		case GLFW_KEY_DOWN:
		{
			dPos -= nForward;
			break;
		}

		case GLFW_KEY_A:
		case GLFW_KEY_LEFT:
		{
			dPos -= nRight;
			break;
		}
		}
	}
	if (glm::length2(dPos) > 0.0f)
	{
		dPos = glm::normalize(dPos);
		m_position += (dPos * dt.assecs() * m_speed);
	}
}
} // namespace le
