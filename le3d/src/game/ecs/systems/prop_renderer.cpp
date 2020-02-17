#include "le3d/core/assert.hpp"
#include "le3d/core/profiler.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/game/ecs.hpp"
#include "le3d/game/ecs/systems/prop_renderer.hpp"
#include "le3d/engine/gfx/utils.hpp"

namespace le
{
void PropRenderer::render(ECSDB const& db) const
{
	std::unordered_map<s64, CompQuery> props;
	{
		// Profiler p("allProps");
		props = db.all<CProp, CTransform>();
	}
	for (auto const& kvp : props)
	{
		auto const& results = kvp.second;
		auto const& pProp = results.get<CProp>();
		auto const& pTransform = results.get<CTransform>();
		if (pProp->m_flags.isSet(CProp::Flag::Wireframe))
		{
			gfx::setPolygonMode(PolygonMode::Line);
		}
		for (auto fixture : pProp->m_fixtures)
		{
			ASSERT(pProp->m_pShader, "null shader!");
			auto const oWorld = fixture.oWorld;
			gfx::Shader::ModelMats mats;
			mats.model = pTransform->m_transform.model();
			mats.normals = pTransform->m_transform.normalModel();
			if (oWorld)
			{
				mats.model *= *oWorld;
				mats.normals *= *oWorld;
			}
			pProp->m_pShader->setModelMats(mats);
			auto const pModel = fixture.pModel;
			auto const pMesh = fixture.pMesh;
			if (pModel)
			{
#if defined(LE3D_DEBUG)
				bool bWasDebug = pModel->m_bDEBUG;
				if (pProp->getOwner()->m_bDebugThis)
				{
					pModel->m_bDEBUG = true;
				}
#endif
				pModel->render(*pProp->m_pShader);
#if defined(LE3D_DEBUG)
				if (pProp->getOwner()->m_bDebugThis)
				{
					pModel->m_bDEBUG = bWasDebug;
				}
#endif
			}
			else if (pMesh)
			{
				pMesh->render(*pProp->m_pShader);
			}
		}
		if (pProp->m_flags.isSet(CProp::Flag::Wireframe))
		{
			gfx::setPolygonMode(PolygonMode::Fill);
		}
	}
	return;
}
} // namespace le
