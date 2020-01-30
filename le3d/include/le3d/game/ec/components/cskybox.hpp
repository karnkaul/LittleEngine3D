#pragma once
#include "le3d/game/resources.hpp"
#include "le3d/game/ec/component.hpp"

namespace le
{
class CSkybox : public Component
{
public:
	static ec::Timing s_timingDelta;

public:
	Skybox m_skybox;
	HShader m_shader;
	Colour m_tint = Colour::White;
	bool m_bAutoDestroySkybox = true;

public:
	CSkybox();
	CSkybox(CSkybox&&);
	CSkybox& operator=(CSkybox&&);
	~CSkybox() override;

public:
	ec::Timing timing() const override;

protected:
	void render() const override;
};
} // namespace le
