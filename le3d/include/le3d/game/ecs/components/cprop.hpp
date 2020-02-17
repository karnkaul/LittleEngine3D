#pragma once
#include <deque>
#include <optional>
#include "le3d/core/colour.hpp"
#include "le3d/engine/gfx/model.hpp"
#include "le3d/game/ecs/component.hpp"

namespace le
{
class CProp : public Component
{
public:
	enum class Flag : u8
	{
		Enabled = 0,
		Wireframe,
		Translucent,
		COUNT_,
	};

	struct Fixture
	{
		gfx::Model const* pModel = nullptr;
		gfx::Mesh const* pMesh = nullptr;
		std::optional<glm::mat4> oWorld;

		Fixture() = default;
		Fixture(gfx::Model const* pModel, std::optional<glm::mat4> oWorld = std::nullopt) : pModel(pModel), oWorld(oWorld) {}
		Fixture(gfx::Mesh const* pMesh, std::optional<glm::mat4> oWorld = std::nullopt) : pMesh(pMesh), oWorld(oWorld) {}
	};

	using Flags = TFlags<Flag>;

public:
	gfx::Shader* m_pShader = nullptr;
	std::deque<Fixture> m_fixtures;
	Flags m_flags;

public:
	void onCreate() override;
};
} // namespace le
