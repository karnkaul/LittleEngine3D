#include <memory>
#include <unordered_map>
#include "le3d/context/context.hpp"
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
std::unordered_map<std::string, HFont> g_fontMap;

HMesh g_debugMesh;
HMesh g_debugQuad;
HMesh g_debugPyramid;
HMesh g_debugTetrahedron;
HMesh g_debugCone;
HMesh g_debugCylinder;
Model g_debugArrow;

HShader g_nullShader;
HTexture g_nullTexture;
HFont g_nullFont;
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

HShader& resources::getShader(const std::string& id)
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

HTexture& resources::loadTexture(std::string id, TexType type, std::vector<u8> bytes, bool bClampToEdge)
{
	if (g_blankTex1px.glID <= 0)
	{
		g_blankTex1px = gfx::gl::genTex("blankTex", type, blank_1pxBytes, false);
	}
	ASSERT(g_textureMap.find(id) == g_textureMap.end(), "Texture already loaded!");
	HTexture texture = gfx::gl::genTex(id, type, std::move(bytes), bClampToEdge);
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

HFont& resources::loadFont(std::string id, HTexture spriteSheet, glm::ivec2 cellsize, glm::ivec2 colsRows, u8 startCode, glm::ivec2 offset)
{
	ASSERT(g_fontMap.find(id) == g_fontMap.end(), "Font already loaded!");
	HFont font = gfx::newFont(std::move(id), std::move(spriteSheet), cellsize);
	if (font.sheet.glID > 0 && font.quad.hVerts.vao > 0)
	{
		font.colsRows = colsRows;
		font.offset = offset;
		font.startCode = startCode;
		g_fontMap.emplace(id, std::move(font));
		return g_fontMap[id];
	}
	return g_nullFont;
}

HFont& resources::getFont(const std::string& id)
{
	auto search = g_fontMap.find(id);
	if (search != g_fontMap.end())
	{
		return search->second;
	}
	return g_nullFont;
}

bool resources::isFontLoaded(const std::string& id)
{
	return g_fontMap.find(id) != g_fontMap.end();
}

bool resources::unload(HFont& font)
{
	auto search = g_fontMap.find(font.name);
	if (search != g_fontMap.end())
	{
		gfx::releaseFonts({&search->second});
		g_fontMap.erase(search);
		return true;
	}
	return false;
}

void resources::unloadFonts()
{
	std::vector<HFont*> fonts;
	for (auto& kvp : g_fontMap)
	{
		fonts.push_back(&kvp.second);
	}
	gfx::releaseFonts(fonts);
	g_fontMap.clear();
}

u32 resources::fontCount()
{
	return (u32)g_fontMap.size();
}

HMesh& resources::debugCube()
{
	if (g_debugMesh.hVerts.vao <= 0)
	{
		g_debugMesh = gfx::createCube(1.0f, "dCube");
	}
	return g_debugMesh;
}

HMesh& resources::debugQuad()
{
	if (g_debugQuad.hVerts.vao <= 0)
	{
		g_debugQuad = gfx::createQuad(1.0f, 1.0f, "dQuad");
	}
	return g_debugQuad;
}

HMesh& resources::debugPyramid()
{
	if (g_debugPyramid.hVerts.vao <= 0)
	{
		g_debugPyramid = gfx::create4Pyramid(1.0f, "dPyramid");
	}
	return g_debugPyramid;
}

HMesh& resources::debugTetrahedron()
{
	if (g_debugTetrahedron.hVerts.vao <= 0)
	{
		g_debugTetrahedron = gfx::createTetrahedron(1.0f, "dTetrahedron");
	}
	return g_debugTetrahedron;
}

HMesh& resources::debugCone()
{
	if (g_debugCone.hVerts.vao <= 0)
	{
		g_debugCone = gfx::createCone(1.0f, 1.0f, 16, "dCone");
	}
	return g_debugCone;
}

HMesh& resources::debugCylinder()
{
	if (g_debugCylinder.hVerts.vao <= 0)
	{
		g_debugCylinder = gfx::createCylinder(1.0f, 1.0f, 16, "dCylinder");
	}
	return g_debugCylinder;
}

Model& resources::debugArrow(const glm::quat& orientation)
{
	if (g_debugArrow.meshCount() == 0)
	{
		g_debugArrow.setupModel("dArrow");
		glm::mat4 m = glm::toMat4(orientation);
		m = glm::scale(m, glm::vec3(0.02f, 0.02f, 0.5f));
		m = glm::rotate(m, glm::radians(90.0f), g_nRight);
		m = glm::translate(m, g_nUp * 0.5f);
		g_debugArrow.addFixture(debugCylinder(), m);
		m = glm::toMat4(orientation);
		m = glm::translate(m, g_nFront * 0.5f);
		m = glm::rotate(m, glm::radians(90.0f), g_nRight);
		m = glm::scale(m, glm::vec3(0.08f, 0.15f, 0.08f));
		g_debugArrow.addFixture(debugCone(), m);
	}
	return g_debugArrow;
}

void resources::unloadAll()
{
	g_debugArrow.release();
	gfx::releaseMeshes({&g_debugMesh, &g_debugQuad, &g_debugPyramid, &g_debugTetrahedron, &g_debugCone, &g_debugCylinder});
	unloadFonts();
	unloadTextures(true);
	unloadShaders();
}
} // namespace le
