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
#endif
}

void Prop::render(const RenderState& state)
{
	if (m_flags.isSet((s32)Flag::Wireframe))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	const Shader& shader = m_oShader ? *m_oShader : state.shader;
	for (auto& fixture : m_fixtures)
	{
		ASSERT(shader.glID.handle > 0, "null shader!");
#if defined(__arm__)
		// Compensate for lack of uniform initialisation in GLES
		pShader->setV4("tint", Colour::White);
#endif
#if defined(DEBUGGING)
		if (m_bDEBUG)
		{
			// pShader->setV4("tint", Colour::Red);
			fixture.pMesh->m_drawFlags.set((s32)Mesh::Flag::BlankMagenta, true);
		}
#endif
		glm::mat4 m = m_transform.model();
		glm::mat4 nm = m_transform.normalModel();
		if (fixture.oModel)
		{
			m *= *fixture.oModel;
			nm *= *fixture.oModel;
		}
		fixture.pMesh->glDraw(m, nm, state, &shader);
	}
	if (m_flags.isSet((s32)Flag::Wireframe))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#if defined(DEBUGGING)
	if (m_pCube && m_pTetra)
	{
		glm::mat4 m1(1.0f);
		glm::mat4 pm(1.0f);
		const glm::vec3 arrowPointScale = glm::vec3(0.08f, 0.15f, 0.08f);
		m1 *= m_transform.model();
		Shader tinted = resources::findShader("unlit/tinted");
		glDisable(GL_DEPTH_TEST);
		gfx::shading::setV4(tinted, "tint", Colour::Blue);
		glm::mat4 m = glm::scale(m1, glm::vec3(0.02f, 0.02f, 0.5f));
		m = glm::translate(m, g_nFront * 0.5f);
		m_pCube->glDraw(m, m, state, &tinted);
		pm = glm::translate(m1, g_nFront * 0.5f);
		pm = glm::rotate(pm, glm::radians(90.0f), g_nRight);
		pm = glm::scale(pm, glm::vec3(0.08f, 0.15f, 0.08f));
		m_pTetra->glDraw(pm, pm, state, &tinted);
		
		gfx::shading::setV4(tinted, "tint", Colour::Red);
		m = glm::scale(m1, glm::vec3(0.5f, 0.02f, 0.02f));
		m = glm::translate(m, g_nRight * 0.5f);
		m_pCube->glDraw(m, m, state, &tinted);
		pm = glm::translate(m1, g_nRight * 0.5f);
		pm = glm::rotate(pm, glm::radians(90.0f), -g_nFront);
		pm = glm::scale(pm, arrowPointScale);
		m_pTetra->glDraw(pm, pm, state, &tinted);
		
		gfx::shading::setV4(tinted, "tint", Colour::Green);
		glm::mat4 mY = glm::scale(m1, glm::vec3(0.02f, 0.5f, 0.02f));
		mY = glm::translate(mY, g_nUp * 0.5f);
		m_pCube->glDraw(mY, mY, state, &tinted);
		pm = glm::translate(m1, g_nUp * 0.5f);
		pm = glm::scale(pm, arrowPointScale);
		m_pTetra->glDraw(pm, pm, state, &tinted);
		glEnable(GL_DEPTH_TEST);
	}
#endif
}

void Prop::addFixture(Mesh& mesh, std::optional<glm::mat4> model /* = std::nullopt */)
{
	m_fixtures.emplace_back(Fixture{&mesh, model});
}

void Prop::clearFixtures()
{
	m_fixtures.clear();
}

void Prop::setShader(Shader shader)
{
	m_oShader.emplace(std::move(shader));
}

void Prop::unsetShader()
{
	m_oShader.reset();
}
} // namespace le
