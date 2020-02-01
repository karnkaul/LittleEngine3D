#include "le3d/game/ecs.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/game/ecs/systems/skyboxRenderer.hpp"

namespace le
{
ecs::Timing SkyboxRenderer::s_timingDelta = -100.0f;

ecs::Timing SkyboxRenderer::timing() const
{
	return m_defaultTiming + s_timingDelta;
}

void SkyboxRenderer::render(ECSDB const& db) const
{
	auto skyboxes = db.all<CSkybox>();
	for (auto const& kvp : skyboxes)
	{
		auto const& results = kvp.second;
		auto const& pSkybox = results.get<CSkybox>();
		if (pSkybox->m_skybox.hCube.glID > 0 && pSkybox->m_shader.glID > 0)
		{
			renderSkybox(pSkybox->m_skybox, pSkybox->m_shader, pSkybox->m_tint);
		}
	}
}
} // namespace le
