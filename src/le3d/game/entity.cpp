#include <assert.h>
#include <glad/glad.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/mesh.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/gfx/utils.hpp"

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
	m_pCube = &resources::debugMesh();
	m_pTetra = &resources::debugTetrahedron();
	m_arrow = Model::debugArrow(g_qIdentity);
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
		pShader->setV4("tint", Colour::White);
#endif
#if defined(DEBUGGING)
		if (m_bDEBUG)
		{
			// pShader->setV4("tint", Colour::Red);
			pModel->m_renderFlags.set((s32)Mesh::Flag::BlankMagenta, true);
		}
#endif
		if (!m_shader.flags.isSet((s32)gfx::shading::Flag::Unlit))
		{
			gfx::shading::setV3(m_shader, "material.ambient", m_untexturedTint.ambient);
			gfx::shading::setV3(m_shader, "material.diffuse", m_untexturedTint.diffuse);
			gfx::shading::setV3(m_shader, "material.specular", m_untexturedTint.specular);
		}
		pModel->render(m_shader, m_transform.model(), m_transform.normalModel());
	}
	if (m_flags.isSet((s32)Flag::Wireframe))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#if defined(DEBUGGING)
	if (m_pCube && m_pTetra)
	{
		glDisable(GL_DEPTH_TEST);
		Shader tinted = resources::findShader("unlit/tinted");
		glm::mat4 mZ = m_transform.model();
		glm::mat4 mX = glm::rotate(mZ, glm::radians(90.0f), g_nUp);
		glm::mat4 mY = glm::rotate(mZ, glm::radians(-90.0f), g_nRight);
		gfx::shading::setV4(tinted, "tint", Colour::Red);
		m_arrow.render(tinted, mX);
		gfx::shading::setV4(tinted, "tint", Colour::Green);
		m_arrow.render(tinted, mY);
		gfx::shading::setV4(tinted, "tint", Colour::Blue);
		m_arrow.render(tinted, mZ);
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

void Prop::setShader(Shader shader)
{
	m_shader = std::move(shader);
}
} // namespace le
