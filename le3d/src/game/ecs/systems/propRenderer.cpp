#include "le3d/core/assert.hpp"
#include "le3d/core/profiler.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/game/ecs.hpp"
#include "le3d/game/ecs/systems/propRenderer.hpp"

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
			context::setPolygonMode(context::PolygonMode::Line);
		}
		for (auto pModel : pProp->m_models)
		{
			ASSERT(pProp->m_shader.glID > 0, "null shader!");
			pProp->m_shader.setV4(env::g_config.uniforms.tint, Colour::White);
#if defined(DEBUGGING)
			if (pProp->getOwner()->m_bDebugThis)
			{
				pModel->m_renderFlags.set(DrawFlag::BlankMagenta, true);
			}
#endif
			ModelMats mats;
			mats.model = pTransform->m_transform.model();
			mats.oNormals = pTransform->m_transform.normalModel();
			pModel->render(pProp->m_shader, mats, pProp->m_oTint ? *pProp->m_oTint : Colour::White);
		}
		if (pProp->m_flags.isSet(CProp::Flag::Wireframe))
		{
			context::setPolygonMode(context::PolygonMode::Fill);
		}
	}
}
} // namespace le
