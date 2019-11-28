#include <assert.h>
#include <glad/glad.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/game/entity.hpp"
#if defined(DEBUGGING)
#include "le3d/game/resources.hpp"
#endif

namespace le
{
void Entity::render(const RenderState& /*state*/) {}

bool Entity::isEnabled() const
{
	return m_flags.isSet((s32)Flag::Enabled);
}

void Entity::setEnabled(bool bEnabled)
{
	m_flags.set((s32)Flag::Enabled, bEnabled);
}

Prop::Prop()
{
#if defined(DEBUGGING)
	m_pArrow = &resources::debugArrow(g_qIdentity);
#endif
}

void Prop::render(const RenderState& state)
{
	if (m_flags.isSet((s32)Flag::Wireframe))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	gfx::shading::setViewMats(m_shader, state.view, state.projection);
	for (auto pModel : m_models)
	{
		ASSERT(m_shader.glID.handle > 0, "null shader!");
#if defined(__arm__)
		// Compensate for lack of uniform initialisation in GLES
		gfx::shading::setV4(m_shader, "tint", Colour::White);
#endif
#if defined(DEBUGGING)
		if (m_bDEBUG)
		{
			// pShader->setV4("tint", Colour::Red);
			pModel->m_renderFlags.set((s32)DrawFlag::BlankMagenta, true);
		}
#endif
		if (!m_shader.flags.isSet((s32)gfx::shading::Flag::Unlit))
		{
			gfx::shading::setV3(m_shader, "material.ambient", m_untexturedTint.ambient);
			gfx::shading::setV3(m_shader, "material.diffuse", m_untexturedTint.diffuse);
			gfx::shading::setV3(m_shader, "material.specular", m_untexturedTint.specular);
		}
		Colour tint;
		if (m_oTintOverride)
		{
			tint = pModel->m_tint;
			pModel->m_tint = *m_oTintOverride;
		}
		pModel->render(m_shader, m_transform.model(), m_transform.normalModel());
		if (m_oTintOverride)
		{
			pModel->m_tint = tint;
		}
	}
	if (m_flags.isSet((s32)Flag::Wireframe))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#if defined(DEBUGGING)
	if (m_pArrow)
	{
		glDisable(GL_DEPTH_TEST);
		HShader tinted = resources::getShader("unlit/tinted");
		glm::mat4 mZ = m_transform.model();
		glm::mat4 mX = glm::rotate(mZ, glm::radians(90.0f), g_nUp);
		glm::mat4 mY = glm::rotate(mZ, glm::radians(-90.0f), g_nRight);
		m_pArrow->m_tint = Colour::Red;
		m_pArrow->render(tinted, mX);
		m_pArrow->m_tint = Colour::Green;
		m_pArrow->render(tinted, mY);
		m_pArrow->m_tint = Colour::Blue;
		m_pArrow->render(tinted, mZ);
		glEnable(GL_DEPTH_TEST);
	}
#endif
}

void Prop::addModel(Model& model)
{
	m_models.emplace_back(&model);
}

void Prop::clearModels()
{
	m_models.clear();
}

void Prop::setShader(HShader shader)
{
	m_shader = std::move(shader);
}
} // namespace le
