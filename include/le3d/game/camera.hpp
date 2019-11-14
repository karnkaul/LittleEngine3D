#pragma once
#include <bitset>
#include <set>
#include "le3d/core/flags.hpp"
#include "le3d/core/time.hpp"
#include "le3d/core/transform.hpp"
#include "le3d/input/input.hpp"

namespace le
{
class Camera
{
public:
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::quat m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	f32 m_fov = 45.0f;

public:
	Camera();
	Camera(Camera&&);
	Camera& operator=(Camera&&);
	virtual ~Camera();

public:
	virtual void tick(Time dt);

public:
	glm::mat4 view() const;
	glm::mat4 perspectiveProj(f32 aspect, f32 near = 0.1f, f32 far = 100.0f) const;
	glm::mat4 orthographicProj(glm::vec4 lrbt, f32 near = 0.1f, f32 far = 100.0f) const;
};

class FreeCam : public Camera
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

public:
	Flags<(size_t)Flag::_COUNT> m_flags;
	f32 m_defaultSpeed = 2.0f;
	f32 m_speed = m_defaultSpeed;
	f32 m_minSpeed = 1.0f;
	f32 m_maxSpeed = 1000.0f;
	f32 m_lookSens = 20.0f;

protected:
	glm::vec2 m_cursorPos = glm::vec2(0.0f);
	glm::vec2 m_nextCursorPos = glm::vec2(0.0f);
	f32 m_minCursorDPosSqr = 0.2f;
	f32 m_pitch = 0.0f;
	f32 m_yaw = 0.0f;
	f32 m_dSpeed = 0.0f;
	bool m_bTicked = false;

private:
	std::set<s32> m_heldKeys;
	OnInput::Token m_tMove;
	OnMouse::Token m_tLook;
	OnMouse::Token m_tZoom;
	OnFocus::Token m_tFocus;

public:
	FreeCam();

	void tick(Time dt) override;
};
} // namespace le
