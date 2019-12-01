#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/game/camera.hpp"
#include "le3d/gfx/gfxtypes.hpp"
#include "le3d/game/scene.hpp"

namespace le
{
RenderState Scene::perspective() const
{
	ASSERT(cameras.uMain.get(), "No main camera!");
	RenderState ret = initState();
	ret.projection = cameras.uMain->perspectiveProj();
	ret.view = cameras.uMain->view();
	return ret;
}

RenderState Scene::orthographic(f32 zoom) const
{
	ASSERT(cameras.uMain.get(), "No main camera!");
	RenderState ret = initState();
	ret.projection = cameras.uMain->orthographicProj(zoom);
	ret.view = cameras.uMain->view();
	return ret;
}

RenderState Scene::initState() const
{
	RenderState state;
	state.dirLights.push_back(lighting.dirLight);
	std::copy(lighting.ptLights.begin(), lighting.ptLights.end(), std::back_inserter(state.ptLights));
	return state;
}
} // namespace le
