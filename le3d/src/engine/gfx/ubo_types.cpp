#include "le3d/core/assert.hpp"
#include "le3d/engine/gfx/ubo_types.hpp"

namespace le::gfx::ubo
{
namespace
{
glm::vec4 padVec4(glm::vec3 const& value, f32 pad = 1.0f)
{
	return glm::vec4(value, pad);
}
} // namespace

stdfs::path const Matrices::s_name = "ubos/matrices";

void Matrices::setViewPos(glm::vec3 const& pos)
{
	viewPos = glm::vec4(pos, 1.0f);
}

stdfs::path const Lights::s_name = "ubos/lights";

void Lights::setLights(std::vector<Data> const& lights)
{
	for (auto const& data : lights)
	{
		if (data.bDirLight)
		{
			ASSERT(data.idx < dirLights.size(), "Invalid index!");
			auto& light = dirLights.at(data.idx);
			light.ambient = padVec4(data.albedo.ambient);
			light.diffuse = padVec4(data.albedo.diffuse);
			light.specular = padVec4(data.albedo.specular);
			light.direction = padVec4(data.direction);
		}
		else
		{
			ASSERT(data.idx < ptLights.size(), "Invalid index!");
			auto& light = ptLights.at(data.idx);
			light.ambient = padVec4(data.albedo.ambient);
			light.diffuse = padVec4(data.albedo.diffuse);
			light.specular = padVec4(data.albedo.specular);
			light.clq = padVec4(data.clq);
			light.position = padVec4(data.position);
		}
	}
}
} // namespace le::gfx::ubo
