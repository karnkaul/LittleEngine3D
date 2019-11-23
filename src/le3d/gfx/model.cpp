#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/model.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/game/resources.hpp"

namespace le
{
Model::Model() = default;
Model::Model(Model&&) = default;
Model& Model::operator=(Model&&) = default;

Model Model::debugArrow(const glm::quat& orientation)
{
	Model arrow;
	arrow.setupModel("arrow");
	glm::mat4 m = glm::toMat4(orientation);
	m = glm::scale(m, glm::vec3(0.02f, 0.02f, 0.5f));
	m = glm::translate(m, g_nFront * 0.5f);
	arrow.addFixture(resources::debugMesh(), m);
	m = glm::toMat4(orientation);
	m = glm::translate(m, g_nFront * 0.5f);
	m = glm::rotate(m, glm::radians(90.0f), g_nRight);
	m = glm::scale(m, glm::vec3(0.08f, 0.15f, 0.08f));
	arrow.addFixture(resources::debugTetrahedron(), m);
	return arrow;
}

Model::~Model()
{
	m_loadedMeshes.clear();
	LOGIF_D(!m_fixtures.empty(), "[%s] %s destroyed", m_name.data(), m_type.data());
}

void Model::addFixture(const Mesh& mesh, std::optional<glm::mat4> model /* = std::nullopt */)
{
	m_fixtures.emplace_back(Fixture{&mesh, model});
}

void Model::setupModel(std::string name)
{
	m_name = std::move(name);
	m_type = Typename(*this);
	LOG_D("[%s] %s setup", m_name.data(), m_type.data());
}

void Model::render(const Shader& shader, const glm::mat4& model, std::optional<glm::mat4> normals /* = std::nullopt */)
{
	ASSERT(shader.glID.handle > 0, "null shader!");
	for (auto& fixture : m_fixtures)
	{
		ASSERT(fixture.pMesh, "Mesh is null!");
#if defined(__arm__)
		// Compensate for lack of uniform initialisation in GLES
		pShader->setV4("tint", Colour::White);
#endif
#if defined(DEBUGGING)
		fixture.pMesh->m_drawFlags = m_renderFlags;
#endif
		glm::mat4 m = model;
		glm::mat4 nm = normals ? *normals : model;
		if (fixture.oWorld)
		{
			m *= *fixture.oWorld;
			nm *= *fixture.oWorld;
		}
		gfx::shading::setModelMats(shader, m, nm);
		fixture.pMesh->draw(shader);
#if defined(DEBUGGING)
		m_renderFlags.flags.reset();
#endif
	}
}
} // namespace le
