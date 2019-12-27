#include <assert.h>
#include <glad/glad.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/game/resources.hpp"
#if defined(DEBUGGING)
#include "le3d/game/utils.hpp"
#endif
#if defined(__arm__)
#include "le3d/env/env.hpp"
#endif

namespace le
{
void Entity::render() {}

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
	m_pArrow = &debug::debugArrow();
#endif
}

void Prop::render()
{
	if (m_flags.isSet((s32)Flag::Wireframe))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	for (auto pModel : m_models)
	{
		ASSERT(m_shader.glID.handle > 0, "null shader!");
#if defined(__arm__)
		// Compensate for lack of uniform initialisation in GLES
		m_shader.setV4(env::g_config.uniforms.tint, Colour::White);
#endif
#if defined(DEBUGGING)
		if (m_bDEBUG)
		{
			pModel->m_renderFlags.set((s32)DrawFlag::BlankMagenta, true);
		}
#endif
		Colour tint;
		if (m_oTintOverride)
		{
			tint = pModel->m_tint;
			pModel->m_tint = *m_oTintOverride;
		}
		ModelMats mats;
		mats.model = m_transform.model();
		mats.oNormals = m_transform.normalModel();
		pModel->render(m_shader, mats);
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
		HShader tinted = resources::get<HShader>("unlit/tinted");
		glm::mat4 mZ = m_transform.model();
		glm::vec3 scale = m_transform.worldScl();
		mZ = glm::scale(mZ, {1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z});
		glm::mat4 mX = glm::rotate(mZ, glm::radians(90.0f), g_nUp);
		glm::mat4 mY = glm::rotate(mZ, glm::radians(-90.0f), g_nRight);
		ModelMats mats;
		mats.model = mX;
		m_pArrow->m_tint = Colour::Red;
		m_pArrow->render(tinted, mats);
		mats.model = mY;
		m_pArrow->m_tint = Colour::Green;
		m_pArrow->render(tinted, mats);
		mats.model = mZ;
		m_pArrow->m_tint = Colour::Blue;
		m_pArrow->render(tinted, mats);
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
