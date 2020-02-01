#pragma once
#include "le3d/game/resources.hpp"
#include "le3d/game/ecs/component.hpp"

namespace le
{
class CSkybox : public Component
{
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
};
} // namespace le
