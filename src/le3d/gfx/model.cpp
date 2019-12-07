#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
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

void Model::render(const HShader& shader, const ModelMats& mats)
{
	ASSERT(shader.glID.handle > 0, "null shader!");
	for (auto& fixture : m_fixtures)
	{
		ASSERT(fixture.pMesh, "Mesh is null!");
		gfx::shading::setV4(shader, env::g_config.uniforms.tint, m_tint);
#if defined(DEBUGGING)
		fixture.pMesh->drawFlags = m_renderFlags;
#endif
		if (fixture.oWorld)
		{
			ModelMats matsCopy = mats;
			matsCopy.model *= *fixture.oWorld;
			if (matsCopy.oNormals)
			{
				*matsCopy.oNormals *= *fixture.oWorld;
			}
			gfx::shading::setModelMats(shader, matsCopy);
		}
		else
		{
			gfx::shading::setModelMats(shader, mats);
		}
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
