#include <assert.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/draw.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/env/env.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/game/resources.hpp"
#if defined(DEBUGGING)
#include "le3d/game/utils.hpp"
#endif

namespace le
{
#if defined(DEBUGGING)
HShader Entity::s_gizmoShader;
#endif

Entity::Entity()
{
#if defined(DEBUGGING)
	m_pArrow = &debug::Arrow();
#endif
}

void Entity::render() const
{
#if defined(DEBUGGING)
	if (m_pArrow && s_gizmoShader.glID.handle > 0)
	{
		context::toggle(context::GFXFlag::DepthTest, false);
		glm::mat4 mZ = m_transform.model();
		glm::vec3 scale = m_transform.worldScl();
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
#endif
}

bool Entity::isEnabled() const
{
	return m_flags.isSet(Flag::Enabled);
}

void Entity::setEnabled(bool bEnabled)
{
	m_flags.set(Flag::Enabled, bEnabled);
}

void Prop::render() const
{
	if (m_flags.isSet(Flag::Wireframe))
	{
		context::setPolygonMode(context::PolygonMode::Line);
	}
	for (auto pModel : m_models)
	{
		ASSERT(m_shader.glID.handle > 0, "null shader!");
		m_shader.setV4(env::g_config.uniforms.tint, Colour::White);
#if defined(DEBUGGING)
		if (m_bDEBUG)
		{
			pModel->m_renderFlags.set(DrawFlag::BlankMagenta, true);
		}
#endif
		ModelMats mats;
		mats.model = m_transform.model();
		mats.oNormals = m_transform.normalModel();
		pModel->render(m_shader, mats, m_oTint ? *m_oTint : Colour::White);
	}
	if (m_flags.isSet(Flag::Wireframe))
	{
		context::setPolygonMode(context::PolygonMode::Fill);
	}
	Entity::render();
}

void Prop::addModel(Model const& model)
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
