#include <assert.h>
#include <glad/glad.h>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/mesh.hpp"
#include "le3d/gfx/shading.hpp"

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
