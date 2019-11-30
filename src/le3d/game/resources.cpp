#include <memory>
#include <unordered_map>
#include "le3d/context/context.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/core/log.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/game/resources.hpp"

namespace le
{
namespace debug
{
extern void unloadAll();
}

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

HShader g_nullShader;
HTexture g_nullTexture;
HFont g_nullFont;

HUBO g_matUBO;
} // namespace

namespace resources
{
HTexture g_blankTex1px;
}

void FontAtlasData::deserialise(std::string json)
{
	GData data(std::move(json));
	cellSize = {data.getS32("cellX", cellSize.x), data.getS32("cellY", cellSize.y)};
	offset = {data.getS32("offsetX", offset.x), data.getS32("offsetY", offset.y)};
	colsRows = {data.getS32("cols", colsRows.x), data.getS32("rows", colsRows.y)};
	startCode = (u8)data.getS32("startCode", startCode);
}

HUBO& resources::matricesUBO()
{
	if (g_matUBO.ubo.handle == 0)
	{
		g_matUBO = gfx::gl::genUBO(2 * sizeof(glm::mat4), 0, gfx::Draw::Static);
	}
	return g_matUBO;
}

HShader& resources::loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags)
{
	ASSERT(g_shaderMap.find(id) == g_shaderMap.end(), "Shader ID already loaded!");
	HShader shader = gfx::gl::genShader(id, vertCode, fragCode, flags);
	if (shader.glID.handle > 0)
	{
		HUBO& matrices = matricesUBO();
		gfx::shading::bindUBO(shader, "Matrices", matrices);
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
		const HShader& shader = kvp.second;
		if (!shader.flags.isSet((s32)gfx::shading::Flag::Unlit))
		{
			gfx::shading::setupLights(kvp.second, dirLights, ptLights);
		}
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

HFont& resources::loadFont(std::string id, FontAtlasData atlas)
{
	ASSERT(g_fontMap.find(id) == g_fontMap.end(), "Font already loaded!");
	HFont font = gfx::newFont(std::move(id), std::move(atlas.bytes), atlas.cellSize);
	if (font.sheet.glID > 0 && font.quad.hVerts.vao > 0)
	{
		font.colsRows = atlas.colsRows;
		font.offset = atlas.offset;
		font.startCode = atlas.startCode;
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

void resources::unloadAll()
{
	debug::unloadAll();
	unloadFonts();
	unloadTextures(true);
	gfx::gl::releaseUBO(g_matUBO);
	unloadShaders();
}
} // namespace le
