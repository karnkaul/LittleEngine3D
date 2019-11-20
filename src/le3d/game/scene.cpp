#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/gfx/gfxtypes.hpp"
#include "le3d/game/scene.hpp"

namespace le
{
RenderState Scene::perspective(f32 aspect) const
{
	ASSERT(cameras.uMain.get(), "No main camera!");
	RenderState ret = initState();
	ret.projection = cameras.uMain->perspectiveProj(aspect);
	ret.view = cameras.uMain->view();
	return ret;
}

RenderState Scene::orthographic(glm::vec4 lrbt) const
{
	ASSERT(cameras.uMain.get(), "No main camera!");
	RenderState ret = initState();
	ret.projection = cameras.uMain->orthographicProj(lrbt);
	ret.view = cameras.uMain->view();
	return ret;
}

RenderState Scene::initState() const
{
	RenderState state;
	state.dirLights.push_back(lighting.dirLight);
	std::copy(lighting.pointLights.begin(), lighting.pointLights.end(), std::back_inserter(state.pointLights));
	state.shader = mainShader;
	return state;
}
} // namespace le
