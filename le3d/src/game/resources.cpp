#include <memory>
#include <unordered_map>
#include "le3d/context/context.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/core/jobs.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"

namespace le
{
namespace
{
std::unordered_map<std::string, HShader> g_shaderMap;
std::unordered_map<std::string, HTexture> g_textureMap;
std::unordered_map<std::string, HFont> g_fontMap;
std::unordered_map<std::string, HUBO> g_uboMap;
std::unordered_map<std::string, Model> g_modelMap;

HShader g_nullShader;
HFont g_nullFont;
HUBO g_nullUBO;
Model g_nullModel;
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

HUBO& resources::addUBO(std::string id, s64 size, u32 bindingPoint, gfx::Draw type)
{
	ASSERT(g_uboMap.find(id) == g_uboMap.end(), "UBO ID already loaded");
	HUBO hUBO = gfx::gl::genUBO(size, bindingPoint, type);
	if (hUBO.ubo.handle > 0)
	{
		g_uboMap.emplace(id, hUBO);
		auto size = utils::friendlySize(hUBO.byteCount);
		LOG_I("== [%s] [%.1f%s] UBO (%d) added for future shaders", id.data(), size.first, size.second.data(), hUBO.bindingPoint);
		return g_uboMap[id];
	}
	return g_nullUBO;
}

template <>
HUBO& resources::get<HUBO>(std::string const& id)
{
	auto search = g_uboMap.find(id);
	if (search != g_uboMap.end())
	{
		return search->second;
	}
	return g_nullUBO;
}

template <>
bool resources::isLoaded<HUBO>(std::string const& id)
{
	return g_uboMap.find(id) != g_uboMap.end();
}

template <>
bool resources::unload<HUBO>(HUBO& hUBO)
{
	auto search = g_uboMap.begin();
	for (; search != g_uboMap.end(); ++search)
	{
		if (search->second.ubo.handle == hUBO.ubo.handle)
		{
			break;
		}
	}
	if (search != g_uboMap.end())
	{
		std::string id = search->first;
		g_uboMap.erase(search);
		auto size = utils::friendlySize(hUBO.byteCount);
		LOG_I("-- [%s] [%.1f%s] UBO (%d) destroyed", id.data(), size.first, size.second.data(), hUBO.bindingPoint);
		gfx::gl::releaseUBO(hUBO);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<HUBO>()
{
	for (auto& kvp : g_uboMap)
	{
		auto& hUBO = kvp.second;
		auto size = utils::friendlySize(hUBO.byteCount);
		LOG_I("-- [%s] [%.1f%s] UBO (%d) destroyed", kvp.first.data(), size.first, size.second.data(), hUBO.bindingPoint);
		gfx::gl::releaseUBO(hUBO);
	}
	g_uboMap.clear();
}

template <>
u32 resources::count<HUBO>()
{
	return (u32)g_uboMap.size();
}

HShader& resources::loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags)
{
	ASSERT(g_shaderMap.find(id) == g_shaderMap.end(), "Shader ID already loaded!");
	HShader shader = gfx::gl::genShader(id, vertCode, fragCode, flags);
	if (shader.glID.handle > 0)
	{
		for (auto const& kvp : g_uboMap)
		{
			shader.bindUBO(kvp.first, kvp.second);
		}
		g_shaderMap.emplace(id, std::move(shader));
		return g_shaderMap[id];
	}
	ASSERT(false, "Failed to load shader!");
	return g_nullShader;
}

template <>
HShader& resources::get<HShader>(std::string const& id)
{
	ASSERT(isLoaded<HShader>(id), "Shader not loaded!");
	if (isLoaded<HShader>(id))
	{
		return g_shaderMap[id];
	}
	return g_nullShader;
}

template <>
bool resources::isLoaded<HShader>(std::string const& id)
{
	return g_shaderMap.find(id) != g_shaderMap.end();
}

template <>
bool resources::unload<HShader>(HShader& shader)
{
	auto search = g_shaderMap.find(shader.id);
	if (search != g_shaderMap.end())
	{
		g_shaderMap.erase(search);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<HShader>()
{
	for (auto& kvp : g_shaderMap)
	{
		gfx::gl::releaseShader(kvp.second);
	}
	g_shaderMap.clear();
}

template <>
u32 resources::count<HShader>()
{
	return (u32)g_shaderMap.size();
}

HTexture& resources::loadTexture(std::string id, TexType type, std::vector<u8> bytes, bool bClampToEdge)
{
	if (g_blankTex1px.glID <= 0)
	{
		u8 const whitepx[] = {0xff, 0xff, 0xff};
		g_blankTex1px = gfx::gl::genTexture("blankTex", whitepx, TexType::Diffuse, sizeof(whitepx), 1, 1, false);
		gfx::g_blankTexID = g_blankTex1px.glID;
	}
	ASSERT(g_textureMap.find(id) == g_textureMap.end(), "Texture already loaded!");
	HTexture texture = gfx::gl::genTexture(id, std::move(bytes), type, bClampToEdge);
	if (texture.glID > 0)
	{
		g_textureMap.emplace(id, std::move(texture));
		return g_textureMap[id];
	}
	ASSERT(false, "Failed to load texture!");
	return g_blankTex1px;
}

template <>
HTexture& resources::get<HTexture>(std::string const& id)
{
	ASSERT(isLoaded<HTexture>(id), "Texture not loaded!");
	if (isLoaded<HTexture>(id))
	{
		return g_textureMap[id];
	}
	return g_blankTex1px;
}

template <>
bool resources::isLoaded<HTexture>(std::string const& id)
{
	return g_textureMap.find(id) != g_textureMap.end();
}

template <>
bool resources::unload<HTexture>(HTexture& texture)
{
	auto search = g_textureMap.find(texture.id);
	if (search != g_textureMap.end())
	{
		gfx::gl::releaseTexture({&search->second});
		g_textureMap.erase(search);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<HTexture>()
{
	std::vector<HTexture*> toDel;
	toDel.reserve(g_textureMap.size());
	for (auto& kvp : g_textureMap)
	{
		toDel.push_back(&kvp.second);
	}
	gfx::gl::releaseTexture(toDel);
	g_textureMap.clear();
}

template <>
u32 resources::count<HTexture>()
{
	return (u32)g_textureMap.size();
}

Skybox resources::createSkybox(std::string name, std::array<std::vector<u8>, 6> rludfb)
{
	Skybox ret;
	ret.cubemap = gfx::gl::genCubemap(name + "_map", std::move(rludfb));
	ret.mesh = gfx::createCube(1.0f, name + "_mesh");
	ret.name = std::move(name);
	LOG_D("[%s] Skybox created", ret.name.data());
	return ret;
}

void resources::destroySkybox(Skybox& skybox)
{
	LOG_D("[%s] Skybox destroyed", skybox.name.data());
	gfx::gl::releaseCubemap(skybox.cubemap);
	gfx::releaseMeshes({&skybox.mesh});
	skybox = Skybox();
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

template <>
HFont& resources::get<HFont>(std::string const& id)
{
	auto search = g_fontMap.find(id);
	if (search != g_fontMap.end())
	{
		return search->second;
	}
	return g_nullFont;
}

template <>
bool resources::isLoaded<HFont>(std::string const& id)
{
	return g_fontMap.find(id) != g_fontMap.end();
}

template <>
bool resources::unload<HFont>(HFont& font)
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

template <>
void resources::unloadAll<HFont>()
{
	std::vector<HFont*> toDel;
	toDel.reserve(g_fontMap.size());
	for (auto& kvp : g_fontMap)
	{
		toDel.push_back(&kvp.second);
	}
	gfx::releaseFonts(toDel);
	g_fontMap.clear();
}

Model& resources::loadModel(std::string id, Model::Data const& data, bool bForceOpaque)
{
	ASSERT(g_modelMap.find(id) == g_modelMap.end(), "Model already loaded!");
	Model newModel;
	newModel.setupModel(id, data, bForceOpaque);
	g_modelMap.emplace(id, std::move(newModel));
	return g_modelMap[id];
}

template <>
Model& resources::get<Model>(std::string const& id)
{
	auto search = g_modelMap.find(id);
	if (search != g_modelMap.end())
	{
		return search->second;
	}
	return g_nullModel;
}

template <>
bool resources::isLoaded<Model>(std::string const& id)
{
	return g_modelMap.find(id) != g_modelMap.end();
}

template <>
bool resources::unload<Model>(Model& model)
{
	auto search = g_modelMap.find(model.m_name);
	if (search != g_modelMap.end())
	{
		g_modelMap.erase(search);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<Model>()
{
	g_modelMap.clear();
}

template <>
u32 resources::count<Model>()
{
	return (u32)g_modelMap.size();
}

void resources::unloadAll()
{
	debug::unloadAll();
	unloadAll<Model>();
	unloadAll<HFont>();
	unloadAll<HTexture>();
	if (g_blankTex1px.glID > 0)
	{
		gfx::gl::releaseTexture({&g_blankTex1px});
	}
	unloadAll<HUBO>();
	unloadAll<HShader>();
}
} // namespace le
