#include <algorithm>
#include <unordered_map>
#include "le3d/core/assert.hpp"
#include "le3d/game/ecs.hpp"
#include "le3d/game/utils.hpp"

namespace le
{
CProp* spawnProp(ECSDB& ecdb, std::string name, bool bDebugGizmo)
{
	auto eID = ecdb.spawnEntity<CProp, CTransform>(std::move(name));
	auto pProp = ecdb.getComponent<CProp>(eID);
	if (bDebugGizmo)
	{
#if defined(LE3D_DEBUG)
		ecdb.addComponent<debug::CGizmo>(eID);
#endif
	}
	return pProp;
}
} // namespace le
