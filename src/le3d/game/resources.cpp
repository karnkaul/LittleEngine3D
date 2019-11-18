#include <unordered_map>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/game/resources.hpp"

namespace le
{
namespace
{
std::unordered_map<std::string, Shader> shaderMap;
}

Shader* shaders::loadShader(std::string id, std::string_view vertCode, std::string_view fragCode)
{
	Shader shader;
	if (shader.glSetup(id, vertCode, fragCode))
	{
		shaderMap.emplace(id, std::move(shader));
		return &shaderMap[id];
	}
	return nullptr;
}

Shader* shaders::findShader(const std::string& id)
{
	auto search = shaderMap.find(id);
	if (search != shaderMap.end())
	{
		return &search->second;
	}
	LOG_W("[shaders] %s Shader not found!", id.data());
	return nullptr;
}

Shader& shaders::getShader(const std::string& id)
{
	ASSERT(shaderMap.find(id) != shaderMap.end(), "Shader not present!");
	return shaderMap[id];
}

bool shaders::unload(Shader& shader)
{
	auto search = shaderMap.find(shader.m_id);
	if (search != shaderMap.end())
	{
		shaderMap.erase(search);
		return true;
	}
	return false;
}

void shaders::unloadAll()
{
	shaderMap.clear();
}

u32 shaders::count()
{
	return (u32)shaderMap.size();
}
} // namespace le
