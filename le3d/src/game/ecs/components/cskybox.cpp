#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/game/ecs/components/cskybox.hpp"

namespace le
{
CSkybox::CSkybox() = default;
CSkybox::CSkybox(CSkybox&&) noexcept = default;
CSkybox& CSkybox::operator=(CSkybox&&) noexcept = default;
CSkybox::~CSkybox()
{
	if (m_bAutoDestroySkybox)
	{
		resources::destroySkybox(m_skybox);
	}
}
} // namespace le
