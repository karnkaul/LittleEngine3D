#include <unordered_map>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/game/resources.hpp"

namespace le
{
namespace
{
static const std::vector<u8> blank_1pxBytes = {
	0x42, 0x4D, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x13, 0x0B, 0x00, 0x00,
	0x13, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
	0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80,
	0x80, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00,
	0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00};

std::unordered_map<std::string, Shader> shaderMap;
std::unordered_map<std::string, Texture> textureMap;
} // namespace

namespace resources
{
Texture g_blankTex1px;
}

Shader* resources::loadShader(std::string id, std::string_view vertCode, std::string_view fragCode)
{
	ASSERT(shaderMap.find(id) == shaderMap.end(), "Shader ID already loaded!");
	Shader shader;
	if (shader.glSetup(id, vertCode, fragCode))
	{
		shaderMap.emplace(id, std::move(shader));
		return &shaderMap[id];
	}
	return nullptr;
}

Shader* resources::findShader(const std::string& id)
{
	auto search = shaderMap.find(id);
	if (search != shaderMap.end())
	{
		return &search->second;
	}
	LOG_W("[shaders] %s Shader not found!", id.data());
	return nullptr;
}

Shader& resources::getShader(const std::string& id)
{
	ASSERT(shaderMap.find(id) != shaderMap.end(), "Shader not loaded!");
	return shaderMap[id];
}

bool resources::unload(Shader& shader)
{
	auto search = shaderMap.find(shader.m_id);
	if (search != shaderMap.end())
	{
		shaderMap.erase(search);
		return true;
	}
	return false;
}

void resources::unloadShaders()
{
	shaderMap.clear();
}

u32 resources::shaderCount()
{
	return (u32)shaderMap.size();
}

Texture* resources::loadTexture(std::string id, std::string type, std::vector<u8> bytes)
{
	if (g_blankTex1px.glID <= 0)
	{
		g_blankTex1px = gfx::gl::genTex("blankTex", "diffuse", blank_1pxBytes);
	}
	ASSERT(textureMap.find(id) == textureMap.end(), "Texture already loaded!");
	Texture texture = gfx::gl::genTex(id, type, std::move(bytes));
	if (texture.glID > 0)
	{
		textureMap.emplace(id, std::move(texture));
		return &textureMap[id];
	}
	return nullptr;
}

Texture* resources::findTexture(const std::string& id)
{
	auto search = textureMap.find(id);
	if (search != textureMap.end())
	{
		return &search->second;
	}
	return nullptr;
}

Texture& resources::getTexture(const std::string& id)
{
	ASSERT(textureMap.find(id) != textureMap.end(), "Texture not loaded!");
	return textureMap[id];
}

bool resources::unload(Texture& texture)
{
	auto search = textureMap.find(texture.id);
	if (search != textureMap.end())
	{
		gfx::gl::releaseTex({&search->second});
		textureMap.erase(search);
		return true;
	}
	return false;
}

void resources::unloadTextures(bool bUnloadBlankTex)
{
	std::vector<Texture*> toDel;
	toDel.reserve(textureMap.size());
	for (auto& kvp : textureMap)
	{
		toDel.push_back(&kvp.second);
	}
	if (bUnloadBlankTex && g_blankTex1px.glID >= 0)
	{
		toDel.push_back(&g_blankTex1px);
	}
	gfx::gl::releaseTex(std::move(toDel));
	textureMap.clear();
}

u32 resources::textureCount()
{
	return (u32)textureMap.size();
}
} // namespace le
