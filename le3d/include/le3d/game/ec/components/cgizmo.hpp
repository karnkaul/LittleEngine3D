#pragma once
#include "le3d/engine/gfx/gfxtypes.hpp"
#include "le3d/game/ec/component.hpp"

namespace le
{
namespace debug
{
class DArrow;
}

class CGizmo : public Component
{
public:
	static ec::Timing s_gizmoTiming;
	static HShader s_gizmoShader;

protected:
	debug::DArrow* m_pArrow = nullptr;

public:
	ec::Timing timing() const override;

protected:
	void onCreate() override;
	void render() const override;
};
} // namespace le
