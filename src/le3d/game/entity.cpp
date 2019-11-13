#include <assert.h>
#include <glad/glad.h>
#include "le3d/core/log.hpp"
#include "le3d/game/entity.hpp"
#include "le3d/gfx/mesh.hpp"
#include "le3d/gfx/shader.hpp"

namespace le
{
Entity::Entity() = default;
Entity::Entity(Entity&&) = default;
Entity& Entity::operator=(Entity&&) = default;
Entity::~Entity()
{
	if (!m_name.empty())
	{
		LOG_D("[%s] %s destroyed", m_name.data(), m_type.data());
	}
}

void Entity::render(const RenderState& /*state*/) {}

void Entity::setup(std::string name)
{
	m_name = std::move(name);
	m_type = Typename(*this);
	LOG_D("[%s] %s set up", m_name.data(), m_type.data());
}
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
	Shader* pShader = m_flags.isSet((s32)Flag::ForceShader) ? m_pShader : state.pShader;
	for (auto& fixture : m_fixtures)
	{
		assert(pShader && "null shader!");
#if defined(DEBUGGING)
		if (m_bDEBUG)
		{
			pShader->setV4("tint", 1.0f, 0.0f, 0.0f, 0.0f);
		}
#endif
		fixture.pMesh->draw(m_transform.model(), state.view, state.projection, *pShader);
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

void Prop::setShader(Shader* pShader, bool bForce)
{
	m_pShader = pShader;
	m_flags.set((s32)Flag::ForceShader, bForce);
}
} // namespace le
