#pragma once
#include "le3d/game/ecs/system.hpp"

namespace le
{
class PropRenderer : public System
{
protected:
	void render(ECSDB const& db) const override;
};
} // namespace le
