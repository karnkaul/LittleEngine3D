#pragma once
#include "le3d/game/ecs/system.hpp"

namespace le
{
class GizmoSystem : public System
{
public:
	static ecs::Timing s_timingDelta;

public:
	ecs::Timing timing() const override;

protected:
	void render(ECSDB const& db) const override;
};
} // namespace le
