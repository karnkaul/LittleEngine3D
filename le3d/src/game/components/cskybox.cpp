#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/game/ec/components/cskybox.hpp"

namespace le
{
ec::Timing CSkybox::s_timingDelta = -5.0f;

CSkybox::CSkybox() = default;
CSkybox::CSkybox(CSkybox&&) = default;
CSkybox& CSkybox::operator=(CSkybox&&) = default;
CSkybox::~CSkybox()
{
	if (m_bAutoDestroySkybox)
	{
		resources::destroySkybox(m_skybox);
	}
}

ec::Timing CSkybox::timing() const
{
	return m_defaultTiming + s_timingDelta;
}

void CSkybox::render() const
{
	if (m_skybox.hCube.glID > 0 && m_shader.glID > 0)
	{
		renderSkybox(m_skybox, m_shader, m_tint);
	}
}
} // namespace le
