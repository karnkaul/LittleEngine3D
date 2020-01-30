#include "le3d/engine/context.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/game/ec/ecImpl.hpp"
#include "le3d/game/ec/components/cgizmo.hpp"
#include "le3d/game/ec/components/ctransform.hpp"

namespace le
{
ec::Timing CGizmo::s_gizmoTiming = 50.0f;
HShader CGizmo::s_gizmoShader;

ec::Timing CGizmo::timing() const
{
	return s_gizmoTiming;
}

void CGizmo::onCreate()
{
	m_pArrow = &debug::Arrow();
}

void CGizmo::render() const
{
	auto pTransform = getComponent<CTransform>();
	if (pTransform && m_pArrow && s_gizmoShader.glID > 0)
	{
		context::toggle(context::GFXFlag::DepthTest, false);
		glm::mat4 mZ = pTransform->m_transform.model();
		glm::vec3 scale = pTransform->m_transform.worldScl();
		mZ = glm::scale(mZ, {1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z});
		glm::mat4 mX = glm::rotate(mZ, glm::radians(90.0f), g_nUp);
		glm::mat4 mY = glm::rotate(mZ, glm::radians(-90.0f), g_nRight);
		ModelMats mats;
		mats.model = mX;
		m_pArrow->render(s_gizmoShader, mats, Colour::Red);
		mats.model = mY;
		m_pArrow->render(s_gizmoShader, mats, Colour::Green);
		mats.model = mZ;
		m_pArrow->render(s_gizmoShader, mats, Colour::Blue);
		context::toggle(context::GFXFlag::DepthTest, true);
	}
}
} // namespace le
