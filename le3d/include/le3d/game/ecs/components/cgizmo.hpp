#pragma once
#include "le3d/engine/gfx/gfx_objects.hpp"
#include "le3d/game/ecs/component.hpp"

namespace le::debug
{
class CGizmo : public Component
{
public:
	enum class Tip
	{
		Cone = 0,
		Sphere,
		Cube
	};

	glm::mat4 m_cylinderMat;
	glm::mat4 m_coneMat;
	glm::mat4 m_cubeMat;
	glm::mat4 m_sphereMat;

	gfx::VertexArray* m_pCylinder = nullptr;
	gfx::VertexArray* m_pCone = nullptr;
	gfx::VertexArray* m_pCube = nullptr;
	gfx::VertexArray* m_pSphere = nullptr;

	Tip m_tip = Tip::Cone;

protected:
	void onCreate() override;
};
} // namespace le::debug
