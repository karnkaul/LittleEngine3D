#include <memory>
#include <unordered_map>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/mesh.hpp"
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

std::unordered_map<std::string, Shader> g_shaderMap;
std::unordered_map<std::string, Texture> g_textureMap;

std::unique_ptr<Mesh> g_uDebugMesh;
std::unique_ptr<Mesh> g_uDebugQuad;
std::unique_ptr<Mesh> g_uDebugPyramid;
std::unique_ptr<Mesh> g_uDebugTetrahedron;

Shader g_nullShader;
Texture g_nullTexture;
} // namespace

namespace resources
{
Texture g_blankTex1px;
}

Shader& resources::loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<Shader::MAX_FLAGS> flags)
{
	ASSERT(g_shaderMap.find(id) == g_shaderMap.end(), "Shader ID already loaded!");
	Shader shader = gfx::gl::genShader(id, vertCode, fragCode, flags);
	if (shader.glID.handle > 0)
	{
		g_shaderMap.emplace(id, std::move(shader));
		return g_shaderMap[id];
	}
	ASSERT(false, "Failed to load shader!");
	return g_nullShader;
}

Shader& resources::findShader(const std::string& id)
{
	ASSERT(isShaderLoaded(id), "Shader not loaded!");
	if (isShaderLoaded(id))
	{
		return g_shaderMap[id];
	}
	return g_nullShader;
}

bool resources::isShaderLoaded(const std::string& id)
{
	return g_shaderMap.find(id) != g_shaderMap.end();
}

bool resources::unload(Shader& shader)
{
	auto search = g_shaderMap.find(shader.id);
	if (search != g_shaderMap.end())
	{
		g_shaderMap.erase(search);
		return true;
	}
	return false;
}

void resources::unloadShaders()
{
	for (auto& kvp : g_shaderMap)
	{
		gfx::gl::releaseShader(kvp.second);
	}
	g_shaderMap.clear();
}

u32 resources::shaderCount()
{
	return (u32)g_shaderMap.size();
}

Texture& resources::loadTexture(std::string id, std::string type, std::vector<u8> bytes)
{
	if (g_blankTex1px.glID <= 0)
	{
		g_blankTex1px = gfx::gl::genTex("blankTex", "diffuse", blank_1pxBytes);
	}
	ASSERT(g_textureMap.find(id) == g_textureMap.end(), "Texture already loaded!");
	Texture texture = gfx::gl::genTex(id, type, std::move(bytes));
	if (texture.glID > 0)
	{
		g_textureMap.emplace(id, std::move(texture));
		return g_textureMap[id];
	}
	ASSERT(false, "Failed to load texture!");
	return g_nullTexture;
}

Texture& resources::getTexture(const std::string& id)
{
	ASSERT(isTextureLoaded(id), "Texture not loaded!");
	if (isTextureLoaded(id))
	{
		return g_textureMap[id];
	}
	return g_nullTexture;
}

bool resources::isTextureLoaded(const std::string& id)
{
	return g_textureMap.find(id) != g_textureMap.end();
}

bool resources::unload(Texture& texture)
{
	auto search = g_textureMap.find(texture.id);
	if (search != g_textureMap.end())
	{
		gfx::gl::releaseTex({&search->second});
		g_textureMap.erase(search);
		return true;
	}
	return false;
}

void resources::unloadTextures(bool bUnloadBlankTex)
{
	std::vector<Texture*> toDel;
	toDel.reserve(g_textureMap.size());
	for (auto& kvp : g_textureMap)
	{
		toDel.push_back(&kvp.second);
	}
	if (bUnloadBlankTex && g_blankTex1px.glID > 0)
	{
		toDel.push_back(&g_blankTex1px);
	}
	gfx::gl::releaseTex(std::move(toDel));
	g_textureMap.clear();
}

u32 resources::textureCount()
{
	return (u32)g_textureMap.size();
}

Mesh& resources::debugMesh()
{
	if (!g_uDebugMesh)
	{
		g_uDebugMesh = std::make_unique<Mesh>(Mesh::createCube(1.0f));
	}
	return *g_uDebugMesh;
}

Mesh& resources::debugQuad()
{
	if (!g_uDebugQuad)
	{
		g_uDebugQuad = std::make_unique<Mesh>(Mesh::createQuad(1.0f));
	}
	return *g_uDebugQuad;
}

Mesh& resources::debugPyramid()
{
	if (!g_uDebugPyramid)
	{
		g_uDebugPyramid = std::make_unique<Mesh>(Mesh::create4Pyramid(1.0f));
	}
	return *g_uDebugPyramid;
}

Mesh& resources::debugTetrahedron()
{
	if (!g_uDebugTetrahedron)
	{
		g_uDebugTetrahedron = std::make_unique<Mesh>(Mesh::createTetrahedron(1.0f));
	}
	return *g_uDebugTetrahedron;
}

void resources::unloadAll()
{
	unloadShaders();
	unloadTextures(true);
	g_uDebugMesh = nullptr;
	g_uDebugQuad = nullptr;
	g_uDebugPyramid = nullptr;
	g_uDebugTetrahedron = nullptr;
}
} // namespace le
