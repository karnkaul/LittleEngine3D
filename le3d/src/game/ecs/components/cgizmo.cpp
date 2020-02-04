#include "le3d/engine/context.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/game/ecs/ecsImpl.hpp"
#include "le3d/game/ecs/components/cgizmo.hpp"
#include "le3d/game/ecs/components/ctransform.hpp"

namespace le
{
HShader CGizmo::s_gizmoShader;

void CGizmo::onCreate()
{
	m_pArrow = &debug::Arrow();
	return;
}
} // namespace le
