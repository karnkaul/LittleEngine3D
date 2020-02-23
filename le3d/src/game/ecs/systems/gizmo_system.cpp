#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/gfx_store.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/game/ecs.hpp"
#include "le3d/game/ecs/systems/gizmo_system.hpp"

namespace le::debug
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
		auto const& u = env::g_config.uniforms;
		auto const& results = kvp.second;
		auto pGizmo = results.get<CGizmo>();
		auto pTransform = results.get<CTransform>();
		auto pShader = gfx::GFXStore::instance()->get<gfx::Shader>("shaders/monolithic");
		if (pShader)
		{
			gfx::setFlag(GLFlag::DepthTest, false);
			glm::mat4 mZ = pTransform->m_transform.model();
			glm::vec3 scale = pTransform->m_transform.worldScale();
			mZ = glm::scale(mZ, {1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z});
			glm::mat4 mX = glm::rotate(mZ, glm::radians(90.0f), g_nUp);
			glm::mat4 mY = glm::rotate(mZ, glm::radians(-90.0f), g_nRight);
			auto renderVA = [&](glm::mat4 const& model, gfx::VertexArray const* pVA, Colour tint) {
				gfx::Shader::ModelMats mats;
				mats.model = model;
				pShader->setModelMats(mats);
				pShader->setMaterial({});
				pShader->setV4(u.material.tint, tint);
				pVA->draw(*pShader);
			};
			glm::mat4 tipMat;
			gfx::VertexArray const* pTipVA;
			switch (pGizmo->m_tip)
			{
			case CGizmo::Tip::Cube:
				tipMat = pGizmo->m_cubeMat;
				pTipVA = pGizmo->m_pCube;
				break;
			case CGizmo::Tip::Sphere:
				tipMat = pGizmo->m_sphereMat;
				pTipVA = pGizmo->m_pSphere;
				break;
			default:
			case CGizmo::Tip::Cone:
				tipMat = pGizmo->m_coneMat;
				pTipVA = pGizmo->m_pCone;
				break;
			}
			renderVA(mX * pGizmo->m_cylinderMat, pGizmo->m_pCylinder, colours::Red);
			renderVA(mX * tipMat, pTipVA, colours::Red);
			renderVA(mY * pGizmo->m_cylinderMat, pGizmo->m_pCylinder, colours::Green);
			renderVA(mY * tipMat, pTipVA, colours::Green);
			renderVA(mZ * pGizmo->m_cylinderMat, pGizmo->m_pCylinder, colours::Blue);
			renderVA(mZ * tipMat, pTipVA, colours::Blue);
			gfx::setFlag(GLFlag::DepthTest, true);
		}
	}
	return;
}
} // namespace le::debug
