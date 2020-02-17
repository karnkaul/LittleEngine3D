#pragma once
#include <unordered_set>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "le3d/core/flags.hpp"
#include "le3d/engine/input_types.hpp"
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
	enum class Flag : u8
	{
		Enabled = 0,
		FixedSpeed,
		Tracking,
		Looking,
		COUNT_
	};
	using Flags = TFlags<Flag>;

	struct Config
	{
		f32 defaultSpeed = 2.0f;
		f32 minSpeed = 1.0f;
		f32 maxSpeed = 1000.0f;

		f32 mouseLookSens = 0.1f;
		f32 mouseLookEpsilon = 0.2f;
		f32 padLookSens = 50.0f;
		f32 padStickEpsilon = 0.05f;
	};
	struct State
	{
		std::unordered_set<Key> heldKeys;
		std::pair<glm::vec2, glm::vec2> cursorPos = {{0.0f, 0.0f}, {0.0f, 0.0f}};

		f32 speed = 2.0f;
		f32 dSpeed = 0.0f;
		f32 pitch = 0.0f;
		f32 yaw = 0.0f;

		Flags flags;
	};

public:
	State m_state;
	Config m_config;

private:
	OnInput::Token m_tMove;
	OnMouse::Token m_tLook;
	OnMouse::Token m_tZoom;
	OnFocus::Token m_tFocus;

protected:
	void onCreate() override;
};
} // namespace le
