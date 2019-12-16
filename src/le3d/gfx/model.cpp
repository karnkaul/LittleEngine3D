#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/model.hpp"
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
#if defined(DEBUGGING)
	bool bResetTint = false;
	bool bSkipTextures = false;
#endif
	for (auto& fixture : m_fixtures)
	{
		ASSERT(fixture.pMesh, "Mesh is null!");
		shader.setV4(env::g_config.uniforms.tint, m_tint);
#if defined(DEBUGGING)
		m_renderFlags.set((s32)DrawFlag::BlankMagenta, m_bDEBUG);
		if (m_renderFlags.isSet((s32)DrawFlag::Blank) || m_renderFlags.isSet((s32)DrawFlag::BlankMagenta))
		{
			bResetTint = m_renderFlags.isSet((s32)DrawFlag::BlankMagenta);
			bSkipTextures = true;
			gfx::setBlankTex(shader, 0, bResetTint);
		}
#endif
		if (fixture.oWorld)
		{
			ModelMats matsCopy = mats;
			matsCopy.model *= *fixture.oWorld;
			if (matsCopy.oNormals)
			{
				*matsCopy.oNormals *= *fixture.oWorld;
			}
			shader.setModelMats(matsCopy);
		}
		else
		{
			shader.setModelMats(mats);
		}
#if defined(DEBUGGING)
		if (!bSkipTextures)
		{
#endif
			gfx::setTextures(shader, fixture.pMesh->textures);
#if defined(DEBUGGING)
		}
#endif
		gfx::drawMesh(*fixture.pMesh, shader);
#if defined(DEBUGGING)
		m_renderFlags.flags.reset();
		if (bResetTint)
		{
			shader.setV4(env::g_config.uniforms.tint, Colour::White);
		}
#endif
	}
	gfx::unsetTextures(-1);
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
