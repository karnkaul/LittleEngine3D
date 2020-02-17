#include "le3d/engine/gfx/gfx_store.hpp"
#include "le3d/engine/gfx/primitives.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/game/utils.hpp"
#include "le3d/game/ecs/ecs_impl.hpp"
#include "le3d/game/ecs/components/cgizmo.hpp"
#include "le3d/game/ecs/components/ctransform.hpp"

namespace le::debug
{
void CGizmo::onCreate()
{
	auto pStore = gfx::GFXStore::instance();
	m_pCylinder = pStore->get<gfx::VertexArray>("primitives/cylinder");
	m_pCone = pStore->get<gfx::VertexArray>("primitives/cone");
	m_pCube = pStore->get<gfx::VertexArray>("primitives/cube");
	m_pSphere = pStore->get<gfx::VertexArray>("primitives/cylinder");
	glm::mat4 m(1.0f);
	m = glm::scale(m, glm::vec3(0.02f, 0.02f, 0.5f));
	m = glm::rotate(m, glm::radians(90.0f), g_nRight);
	m_cylinderMat = glm::translate(m, g_nUp * 0.5f);
	m = glm::translate(glm::mat4(1.0f), g_nFront * 0.5f);
	m = glm::rotate(m, glm::radians(90.0f), g_nRight);
	m_coneMat = glm::scale(m, glm::vec3(0.08f, 0.15f, 0.08f));
	m_cubeMat = glm::scale(m, glm::vec3(0.08f, 0.08f, 0.08f));
	m_sphereMat = glm::scale(m, glm::vec3(0.08f, 0.08f, 0.08f));
}
} // namespace le::debug
