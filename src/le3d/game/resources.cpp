#include <memory>
#include <unordered_map>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/primitives.hpp"
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

std::unordered_map<std::string, HShader> g_shaderMap;
std::unordered_map<std::string, HTexture> g_textureMap;

HMesh g_debugMesh;
HMesh g_debugQuad;
HMesh g_debugPyramid;
HMesh g_debugTetrahedron;
Model g_debugArrow;

HShader g_nullShader;
HTexture g_nullTexture;
} // namespace

namespace resources
{
HTexture g_blankTex1px;
}

HShader& resources::loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags)
{
	ASSERT(g_shaderMap.find(id) == g_shaderMap.end(), "Shader ID already loaded!");
	HShader shader = gfx::gl::genShader(id, vertCode, fragCode, flags);
	if (shader.glID.handle > 0)
	{
		g_shaderMap.emplace(id, std::move(shader));
		return g_shaderMap[id];
	}
	ASSERT(false, "Failed to load shader!");
	return g_nullShader;
}

HShader& resources::findShader(const std::string& id)
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

bool resources::unload(HShader& shader)
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

void resources::shadeLights(const std::vector<DirLight>& dirLights, const std::vector<PtLight>& ptLights)
{
	for (const auto& kvp : g_shaderMap)
	{
		gfx::shading::setupLights(kvp.second, dirLights, ptLights);
	}
}

HTexture& resources::loadTexture(std::string id, std::string type, std::vector<u8> bytes)
{
	if (g_blankTex1px.glID <= 0)
	{
		g_blankTex1px = gfx::gl::genTex("blankTex", "diffuse", blank_1pxBytes);
	}
	ASSERT(g_textureMap.find(id) == g_textureMap.end(), "Texture already loaded!");
	HTexture texture = gfx::gl::genTex(id, type, std::move(bytes));
	if (texture.glID > 0)
	{
		g_textureMap.emplace(id, std::move(texture));
		return g_textureMap[id];
	}
	ASSERT(false, "Failed to load texture!");
	return g_nullTexture;
}

HTexture& resources::getTexture(const std::string& id)
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

bool resources::unload(HTexture& texture)
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
	std::vector<HTexture*> toDel;
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

HMesh& resources::debugMesh()
{
	if (g_debugMesh.hVerts.vao <= 0)
	{
		g_debugMesh = gfx::createCube(1.0f);
	}
	return g_debugMesh;
}

HMesh& resources::debugQuad()
{
	if (g_debugQuad.hVerts.vao <= 0)
	{
		g_debugQuad = gfx::createQuad(1.0f);
	}
	return g_debugQuad;
}

HMesh& resources::debugPyramid()
{
	if (g_debugPyramid.hVerts.vao <= 0)
	{
		g_debugPyramid = gfx::create4Pyramid(1.0f);
	}
	return g_debugPyramid;
}

HMesh& resources::debugTetrahedron()
{
	if (g_debugTetrahedron.hVerts.vao <= 0)
	{
		g_debugTetrahedron = gfx::createTetrahedron(1.0f);
	}
	return g_debugTetrahedron;
}

Model& resources::debugArrow(const glm::quat& orientation)
{
	if (g_debugArrow.meshCount() == 0)
	{
		g_debugArrow.setupModel("dArrow");
		glm::mat4 m = glm::toMat4(orientation);
		m = glm::scale(m, glm::vec3(0.02f, 0.02f, 0.5f));
		m = glm::translate(m, g_nFront * 0.5f);
		g_debugArrow.addFixture(resources::debugMesh(), m);
		m = glm::toMat4(orientation);
		m = glm::translate(m, g_nFront * 0.5f);
		m = glm::rotate(m, glm::radians(90.0f), g_nRight);
		m = glm::scale(m, glm::vec3(0.08f, 0.15f, 0.08f));
		g_debugArrow.addFixture(resources::debugTetrahedron(), m);
	}
	return g_debugArrow;
}

void resources::unloadAll()
{
	g_debugArrow.release();
	gfx::releaseMesh(g_debugMesh);
	gfx::releaseMesh(g_debugQuad);
	gfx::releaseMesh(g_debugPyramid);
	gfx::releaseMesh(g_debugTetrahedron);
	unloadShaders();
	unloadTextures(true);
}
} // namespace le
