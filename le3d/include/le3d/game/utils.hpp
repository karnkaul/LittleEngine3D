#pragma once
#include <deque>
#include "le3d/core/colour.hpp"
#include "le3d/core/time.hpp"
#include "le3d/core/std_types.hpp"
#include "le3d/game/ecs/components/cprop.hpp"

namespace le
{
CProp* spawnProp(ECSDB& ecsdb, std::string name, bool bDebugGizmo = true);
} // namespace le
