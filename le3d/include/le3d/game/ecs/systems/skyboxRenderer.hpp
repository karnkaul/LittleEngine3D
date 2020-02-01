#pragma once
#include "le3d/game/ecs/system.hpp"

namespace le
{
class SkyboxRenderer : public System
{
public:
	static ecs::Timing s_timingDelta;

public:
	ecs::Timing timing() const override;

protected:
	void render(class ECSDB const& db) const override;
};
} // namespace le
