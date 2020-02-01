#pragma once
#include "le3d/engine/gfx/gfxtypes.hpp"
#include "le3d/game/ecs/component.hpp"

namespace le
{
namespace debug
{
class DArrow;
}

class CGizmo : public Component
{
public:
	static HShader s_gizmoShader;

	debug::DArrow* m_pArrow = nullptr;

protected:
	void onCreate() override;
};
} // namespace le
