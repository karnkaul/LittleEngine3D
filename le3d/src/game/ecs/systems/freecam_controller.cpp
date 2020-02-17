#include "le3d/core/maths.hpp"
#include "le3d/engine/input.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/game/ecs.hpp"
#include "le3d/game/ecs/systems/freecam_controller.hpp"

namespace le
{
void FreeCamController::tick(ECSDB& db, Time dt)
{
	auto freeCams = db.all<CFreeCam>();
	for (auto& kvp : freeCams)
	{
		auto& results = kvp.second;
		auto pCam = results.get<CFreeCam>();
		if (!pCam->m_state.flags.isSet(CFreeCam::Flag::Enabled))
		{
			return;
		}
		GamepadState pad0 = input::getGamepadState(0);

		// Speed
		if (!pCam->m_state.flags.isSet(CFreeCam::Flag::FixedSpeed))
		{
			if (pad0.isPressed(Key::GAMEPAD_BUTTON_LEFT_BUMPER))
			{
				pCam->m_state.dSpeed -= (dt.assecs() * 10);
			}
			else if (pad0.isPressed(Key::GAMEPAD_BUTTON_RIGHT_BUMPER))
			{
				pCam->m_state.dSpeed += (dt.assecs() * 10);
			}
			if (pCam->m_state.dSpeed * pCam->m_state.dSpeed > 0.0f)
			{
				pCam->m_state.speed = maths::clamp(pCam->m_state.speed + (pCam->m_state.dSpeed * dt.assecs() * 100), pCam->m_config.minSpeed, pCam->m_config.maxSpeed);
				pCam->m_state.dSpeed = maths::lerp(pCam->m_state.dSpeed, 0.0f, 0.75f);
				if (pCam->m_state.dSpeed * pCam->m_state.dSpeed < 0.01f)
				{
					pCam->m_state.dSpeed = 0.0f;
				}
			}
		}

		// Elevation
		f32 elevation =
			input::triggerToAxis(pad0.getAxis(PadAxis::RIGHT_TRIGGER)) - input::triggerToAxis(pad0.getAxis(PadAxis::LEFT_TRIGGER));
		if (elevation * elevation > 0.01f)
		{
			pCam->m_position.y += (elevation * dt.assecs() * pCam->m_state.speed);
		}

		// Look
		f32 dLook = pCam->m_config.padLookSens * dt.assecs();
		glm::vec2 const padRight(pad0.getAxis(PadAxis::RIGHT_X), pad0.getAxis(PadAxis::RIGHT_Y));
		if (glm::length2(padRight) > pCam->m_config.padStickEpsilon)
		{
			pCam->m_state.pitch += (padRight.y * dLook);
			pCam->m_state.yaw += (padRight.x * dLook);
		}

		dLook = pCam->m_config.mouseLookSens;
		glm::vec2 dCursorPos = pCam->m_state.cursorPos.second - pCam->m_state.cursorPos.first;
		if (glm::length2(dCursorPos) > pCam->m_config.mouseLookEpsilon)
		{
			pCam->m_state.yaw += (dCursorPos.x * dLook);
			pCam->m_state.pitch += (dCursorPos.y * dLook);
			pCam->m_state.cursorPos.first = pCam->m_state.cursorPos.second;
		}
		glm::quat pitch = glm::angleAxis(glm::radians(-pCam->m_state.pitch), g_nRight);
		glm::quat yaw = glm::angleAxis(glm::radians(-pCam->m_state.yaw), g_nUp);
		pCam->m_orientation = yaw * pitch;

		// Move
		glm::vec3 dPos = glm::vec3(0.0f);
		glm::vec3 const nForward = -glm::normalize(glm::rotate(pCam->m_orientation, g_nFront));
		glm::vec3 const nRight = glm::normalize(glm::rotate(pCam->m_orientation, g_nRight));
		glm::vec2 const padLeft(pad0.getAxis(PadAxis::LEFT_X), -pad0.getAxis(PadAxis::LEFT_Y));

		if (glm::length2(padLeft) > pCam->m_config.padStickEpsilon)
		{
			dPos += (nForward * padLeft.y);
			dPos += (nRight * padLeft.x);
		}

		for (auto key : pCam->m_state.heldKeys)
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
			pCam->m_position += (dPos * dt.assecs() * pCam->m_state.speed);
		}

		m_forEachCam(*pCam);
	}
	return;
}
} // namespace le
