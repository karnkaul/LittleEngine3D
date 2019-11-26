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

Model::~Model()
{
	release();
}

void Model::addFixture(const HMesh& mesh, std::optional<glm::mat4> model /* = std::nullopt */)
{
	m_fixtures.emplace_back(Fixture{&mesh, model});
}

void Model::setupModel(std::string name)
{
	m_name = std::move(name);
	m_type = Typename(*this);
	LOG_D("[%s] %s setup", m_name.data(), m_type.data());
}

void Model::render(const HShader& shader, const glm::mat4& model, std::optional<glm::mat4> normals /* = std::nullopt */)
{
	ASSERT(shader.glID.handle > 0, "null shader!");
	for (auto& fixture : m_fixtures)
	{
		ASSERT(fixture.pMesh, "Mesh is null!");
		gfx::shading::setV4(shader, "tint", m_tint);
#if defined(DEBUGGING)
		fixture.pMesh->drawFlags = m_renderFlags;
#endif
		glm::mat4 m = model;
		glm::mat4 nm = normals ? *normals : model;
		if (fixture.oWorld)
		{
			m *= *fixture.oWorld;
			nm *= *fixture.oWorld;
		}
		gfx::shading::setModelMats(shader, m, nm);
		gfx::drawMesh(*fixture.pMesh, shader);
#if defined(DEBUGGING)
		m_renderFlags.flags.reset();
#endif
	}
}

u32 Model::meshCount() const
{
	return (u32)m_fixtures.size();
}

void Model::release()
{
	std::vector<HMesh*> toRelease;
	toRelease.reserve(m_loadedMeshes.size());
	for (auto& mesh : m_loadedMeshes)
	{
		toRelease.push_back(&mesh);
	}
	gfx::releaseMeshes(toRelease);
	m_loadedMeshes.clear();
	LOGIF_D(!m_fixtures.empty(), "[%s] %s destroyed", m_name.data(), m_type.data());
	m_fixtures.clear();
	m_name.clear();
}
} // namespace le
