#pragma once
#include <memory>
#include <vector>
#include "le3d/thirdParty.hpp"
#include "le3d/game/object.hpp"

namespace le
{
struct Lighting final
{
	std::vector<PtLight> pointLights;
	DirLight dirLight;
};

struct Cameras final
{
	std::unique_ptr<class Camera> uMain;
};

struct Scene
{
	Lighting lighting;
	Cameras cameras;
	HShader mainShader;

	RenderState initState() const;
	RenderState perspective(f32 aspect) const;
	RenderState orthographic(glm::vec4 lrbt) const;
};
} // namespace le
