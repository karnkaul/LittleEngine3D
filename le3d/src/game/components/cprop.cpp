#include "le3d/core/assert.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/env/env.hpp"
#include "le3d/game/ec/ecImpl.hpp"
#include "le3d/game/ec/components/cprop.hpp"
#include "le3d/game/ec/components/ctransform.hpp"
#if defined(DEBUGGING)
#include "le3d/game/utils.hpp"
#endif

namespace le
{
ec::Timing CProp::s_transparentTimingDelta = 5.0f;

ec::Timing CProp::timing() const
{
	if (m_flags.isSet(Flag::Transparent))
	{
		return m_defaultTiming + s_transparentTimingDelta;
	}
	else
	{
		return m_defaultTiming;
	}
}

void CProp::render() const
{
	if (m_pOwner)
	{
		auto pTransform = getComponent<CTransform>();
		if (m_flags.isSet(CProp::Flag::Wireframe))
		{
			context::setPolygonMode(context::PolygonMode::Line);
		}
		for (auto pModel : m_models)
		{
			ASSERT(m_shader.glID > 0, "null shader!");
			m_shader.setV4(env::g_config.uniforms.tint, Colour::White);
#if defined(DEBUGGING)
			if (m_pOwner->m_bDEBUG)
			{
				pModel->m_renderFlags.set(DrawFlag::BlankMagenta, true);
			}
#endif
			ModelMats mats;
			mats.model = pTransform ? pTransform->m_transform.model() : glm::mat4(1.0f);
			mats.oNormals = pTransform ? pTransform->m_transform.normalModel() : mats.model;
			pModel->render(m_shader, mats, m_oTint ? *m_oTint : Colour::White);
		}
		if (m_flags.isSet(CProp::Flag::Wireframe))
		{
			context::setPolygonMode(context::PolygonMode::Fill);
		}
	}
}
} // namespace le
