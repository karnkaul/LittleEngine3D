#include "le3d/engine/context.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/game/ecs.hpp"
#include "le3d/game/ecs/systems/gizmoSystem.hpp"

namespace le
{
ecs::Timing GizmoSystem::s_timingDelta = 500.0f;

ecs::Timing GizmoSystem::timing() const
{
	return m_defaultTiming + s_timingDelta;
}

void GizmoSystem::render(ECSDB const& db) const
{
	auto gizmos = db.all<CGizmo, CTransform>();
	for (auto const& kvp : gizmos)
	{
		auto const& results = kvp.second;
		auto pGizmo = results.get<CGizmo>();
		auto pTransform = results.get<CTransform>();
		if (pGizmo->m_pArrow && CGizmo::s_gizmoShader.glID > 0)
		{
			context::toggle(context::GFXFlag::DepthTest, false);
			glm::mat4 mZ = pTransform->m_transform.model();
			glm::vec3 scale = pTransform->m_transform.worldScl();
			mZ = glm::scale(mZ, {1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z});
			glm::mat4 mX = glm::rotate(mZ, glm::radians(90.0f), g_nUp);
			glm::mat4 mY = glm::rotate(mZ, glm::radians(-90.0f), g_nRight);
			ModelMats mats;
			mats.model = mX;
			pGizmo->m_pArrow->render(CGizmo::s_gizmoShader, mats, Colour::Red);
			mats.model = mY;
			pGizmo->m_pArrow->render(CGizmo::s_gizmoShader, mats, Colour::Green);
			mats.model = mZ;
			pGizmo->m_pArrow->render(CGizmo::s_gizmoShader, mats, Colour::Blue);
			context::toggle(context::GFXFlag::DepthTest, true);
		}
	}
}
} // namespace le
