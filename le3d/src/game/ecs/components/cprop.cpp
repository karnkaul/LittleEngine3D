#include "le3d/engine/gfx/gfx_store.hpp"
#include "le3d/game/ecs/components/cprop.hpp"

namespace le
{
void CProp::onCreate()
{
	if (!m_pShader)
	{
		m_pShader = gfx::GFXStore::instance()->get<gfx::Shader>("shaders/monolithic");
	}
}
} // namespace le
