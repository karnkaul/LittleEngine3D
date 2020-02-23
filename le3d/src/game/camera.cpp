#include <glm/gtx/quaternion.hpp>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/maths.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/engine/input.hpp"
#include "le3d/game/camera.hpp"

namespace le
{
Camera::Camera() = default;
Camera::Camera(Camera&&) = default;
Camera& Camera::operator=(Camera&&) = default;
Camera::~Camera() = default;

void Camera::tick(Time) {}

bool s_bTEST = false;
glm::mat4 Camera::view() const
{
#if defined(LE3D_DEBUG)
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

glm::mat4 Camera::perspectiveProj(f32 near, f32 far) const
{
	return glm::perspective(glm::radians(m_fov), m_aspectRatio > 0.0f ? m_aspectRatio : context::windowAspect(), near, far);
}

glm::mat4 Camera::orthographicProj(f32 zoom, f32 near, f32 far) const
{
	ASSERT(zoom > 0.0f, "Invalid zoom!");
	f32 ar = m_aspectRatio > 0.0f ? m_aspectRatio : context::windowAspect();
	f32 w = ar > 1.0f ? 1.0f : 1.0f * ar;
	f32 h = ar > 1.0f ? 1.0f / ar : 1.0f;
	return glm::ortho(-w / zoom, w / zoom, -h / zoom, h / zoom, near, far);
}

glm::mat4 Camera::uiProj(glm::vec3 const& uiSpace) const
{
	f32 const w = uiSpace.x * 0.5f;
	f32 const h = uiSpace.y * 0.5f;
	return glm::ortho(-w, w, -h, h, -uiSpace.z, uiSpace.z);
}

FreeCam::FreeCam()
{
	m_state.speed = m_config.defaultSpeed;
	m_tMove = input::registerInput([this](Key key, Action action, Mods /*mods*/) {
		if (m_state.flags.isSet(Flag::Enabled))
		{
			switch (action)
			{
			case Action::PRESS:
			{
				m_state.heldKeys.emplace(key);
				break;
			}
			case Action::RELEASE:
			{
				if (!m_state.flags.isSet(Flag::FixedSpeed) && key == Key::MOUSE_BUTTON_3)
				{
					m_state.speed = m_config.defaultSpeed;
				}
				m_state.heldKeys.erase(key);
				break;
			}
			default:
				break;
			}
			if (key == Key::MOUSE_BUTTON_2)
			{
				bool bLook = action == Action::PRESS;
				if (m_state.flags.isSet(Flag::Looking) ^ bLook)
				{
					m_state.flags.set(Flag::Tracking, false);
				}
				m_state.flags.set(Flag::Looking, bLook);
				input::setCursorMode(bLook ? CursorMode::Disabled : CursorMode::Default);
			}
		}
	});
	m_tLook = input::registerMouse([this](f64 x, f64 y) {
		if (m_state.flags.isSet(Flag::Enabled) && m_state.flags.isSet(Flag::Looking))
		{
			m_state.cursorPos.second = {(f32)x, (f32)y};
			if (!m_state.flags.isSet(Flag::Tracking))
			{
				m_state.cursorPos.first = {(f32)x, (f32)y};
				m_state.flags.set(Flag::Tracking, true);
			}
		}
	});
	m_tZoom = input::registerScroll([this](f64 /*dx*/, f64 dy) { m_state.dSpeed += (f32)dy; });
	m_tFocus = input::registerFocus([this](bool /*bFocus*/) { m_state.flags.set(Flag::Tracking, false); });
	m_state.flags.set(Flag::Enabled, true);
	return;
}

FreeCam::FreeCam(FreeCam&&) = default;
FreeCam& FreeCam::operator=(FreeCam&&) = default;
FreeCam::~FreeCam() = default;

void FreeCam::tick(Time dt)
{
	if (!m_state.flags.isSet(Flag::Enabled))
	{
		return;
	}
	GamepadState pad0 = input::getGamepadState(0);

	// Speed
	if (!m_state.flags.isSet(Flag::FixedSpeed))
	{
		if (pad0.isPressed(Key::GAMEPAD_BUTTON_LEFT_BUMPER))
		{
			m_state.dSpeed -= (dt.to_s() * 10);
		}
		else if (pad0.isPressed(Key::GAMEPAD_BUTTON_RIGHT_BUMPER))
		{
			m_state.dSpeed += (dt.to_s() * 10);
		}
		if (m_state.dSpeed * m_state.dSpeed > 0.0f)
		{
			m_state.speed = maths::clamp(m_state.speed + (m_state.dSpeed * dt.to_s() * 100), m_config.minSpeed, m_config.maxSpeed);
			m_state.dSpeed = maths::lerp(m_state.dSpeed, 0.0f, 0.75f);
			if (m_state.dSpeed * m_state.dSpeed < 0.01f)
			{
				m_state.dSpeed = 0.0f;
			}
		}
	}

	// Elevation
	f32 elevation = input::triggerToAxis(pad0.getAxis(PadAxis::RIGHT_TRIGGER)) - input::triggerToAxis(pad0.getAxis(PadAxis::LEFT_TRIGGER));
	if (elevation * elevation > 0.01f)
	{
		m_position.y += (elevation * dt.to_s() * m_state.speed);
	}

	// Look
	f32 dLook = m_config.padLookSens * dt.to_s();
	glm::vec2 const padRight(pad0.getAxis(PadAxis::RIGHT_X), pad0.getAxis(PadAxis::RIGHT_Y));
	if (glm::length2(padRight) > m_config.padStickEpsilon)
	{
		m_state.pitch += (padRight.y * dLook);
		m_state.yaw += (padRight.x * dLook);
	}

	dLook = m_config.mouseLookSens;
	glm::vec2 dCursorPos = m_state.cursorPos.second - m_state.cursorPos.first;
	if (glm::length2(dCursorPos) > m_config.mouseLookEpsilon)
	{
		m_state.yaw += (dCursorPos.x * dLook);
		m_state.pitch += (dCursorPos.y * dLook);
		m_state.cursorPos.first = m_state.cursorPos.second;
	}
	glm::quat pitch = glm::angleAxis(glm::radians(-m_state.pitch), g_nRight);
	glm::quat yaw = glm::angleAxis(glm::radians(-m_state.yaw), g_nUp);
	m_orientation = yaw * pitch;

	// Move
	glm::vec3 dPos = glm::vec3(0.0f);
	glm::vec3 const nForward = -glm::normalize(glm::rotate(m_orientation, g_nFront));
	glm::vec3 const nRight = glm::normalize(glm::rotate(m_orientation, g_nRight));
	glm::vec2 const padLeft(pad0.getAxis(PadAxis::LEFT_X), -pad0.getAxis(PadAxis::LEFT_Y));

	if (glm::length2(padLeft) > m_config.padStickEpsilon)
	{
		dPos += (nForward * padLeft.y);
		dPos += (nRight * padLeft.x);
	}

	for (auto key : m_state.heldKeys)
	{
		switch (key)
		{
		default:
			break;

		case Key::W:
		case Key::UP:
		{
			dPos += nForward;
			break;
		}

		case Key::D:
		case Key::RIGHT:
		{
			dPos += nRight;
			break;
		}

		case Key::S:
		case Key::DOWN:
		{
			dPos -= nForward;
			break;
		}

		case Key::A:
		case Key::LEFT:
		{
			dPos -= nRight;
			break;
		}
		}
	}
	if (glm::length2(dPos) > 0.0f)
	{
		dPos = glm::normalize(dPos);
		m_position += (dPos * dt.to_s() * m_state.speed);
	}
	return;
}
} // namespace le
