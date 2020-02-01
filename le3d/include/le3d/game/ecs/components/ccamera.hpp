#pragma once
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "le3d/core/tFlags.hpp"
#include "le3d/engine/inputTypes.hpp"
#include "le3d/game/ecs/component.hpp"

namespace le
{
class CCamera : public Component
{
public:
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::quat m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	f32 m_fov = 45.0f;
	f32 m_aspectRatio = 0.0f;

public:
	glm::mat4 view() const;
	glm::mat4 perspectiveProj(f32 near = 0.1f, f32 far = 100.0f) const;
	glm::mat4 orthographicProj(f32 zoom = 1.0f, f32 near = 0.1f, f32 far = 100.0f) const;
	glm::mat4 uiProj(glm::vec3 const& uiSpace) const;
};

class CFreeCam : public CCamera
{
public:
	enum class Flag
	{
		Enabled = 0,
		InitPos,
		FixedSpeed,
		Looking,
		_COUNT
	};

	using Flags = TFlags<(size_t)Flag::_COUNT, Flag>;

public:
	Flags m_flags;
	f32 m_defaultSpeed = 2.0f;
	f32 m_speed = m_defaultSpeed;
	f32 m_minSpeed = 1.0f;
	f32 m_maxSpeed = 1000.0f;
	f32 m_mouseLookSens = 0.1f;
	f32 m_joyLookSens = 50.0f;

	f32 m_minJoyRightDPosSqr = 0.05f;
	f32 m_minCursorDPosSqr = 0.2f;
	f32 m_pitch = 0.0f;
	f32 m_yaw = 0.0f;
	f32 m_dSpeed = 0.0f;
	bool m_bTicked = false;

	glm::vec2 m_cursorPos = glm::vec2(0.0f);
	glm::vec2 m_nextCursorPos = glm::vec2(0.0f);
	std::unordered_set<Key> m_heldKeys;

private:
	OnInput::Token m_tMove;
	OnMouse::Token m_tLook;
	OnMouse::Token m_tZoom;
	OnFocus::Token m_tFocus;

protected:
	void onCreate() override;
};
} // namespace le
