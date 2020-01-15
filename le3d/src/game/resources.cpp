#include <memory>
#include <unordered_map>
#include "le3d/engine/context.hpp"
#include "le3d/core/assert.hpp"
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
std::unordered_map<std::string, BitmapFont> g_fontMap;
std::unordered_map<std::string, HUBO> g_uboMap;
std::unordered_map<std::string, Model> g_modelMap;

HShader g_nullShader;
BitmapFont g_nullFont;
HUBO g_nullUBO;
Model g_nullModel;
} // namespace

HUBO& resources::addUBO(std::string const& id, s64 size, u32 bindingPoint, gfx::Draw type)
{
	ASSERT(g_uboMap.find(id) == g_uboMap.end(), "UBO ID already loaded");
	HUBO hUBO = gfx::genUBO(id, size, bindingPoint, type);
	if (hUBO.glID > 0)
	{
		g_uboMap.emplace(id, std::move(hUBO));
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
	LOG_W("[Resources] [%s] [%s] not found!", id.data(), Typename<HUBO>().data());
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
	auto search = g_uboMap.find(hUBO.id);
	if (search != g_uboMap.end())
	{
		std::string id = search->first;
		g_uboMap.erase(search);
		gfx::releaseUBO(hUBO);
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
		gfx::releaseUBO(hUBO);
	}
	g_uboMap.clear();
}

template <>
u32 resources::count<HUBO>()
{
	return (u32)g_uboMap.size();
}

HShader& resources::loadShader(std::string const& id, std::string_view vertCode, std::string_view fragCode)
{
	ASSERT(g_shaderMap.find(id) == g_shaderMap.end(), "Shader ID already loaded!");
	HShader shader = gfx::genShader(id, vertCode, fragCode);
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
	LOG_E("[Resources] [%s] Failed to load [%s]!", Typename<HShader>().data(), id.data());
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
	LOG_W("[Resources] [%s] [%s] not found!", id.data(), Typename<HShader>().data());
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
		gfx::releaseShader(kvp.second);
	}
	g_shaderMap.clear();
}

template <>
u32 resources::count<HShader>()
{
	return (u32)g_shaderMap.size();
}

HTexture& resources::loadTexture(std::string const& id, TexType type, bytestream bytes, bool bClampToEdge)
{
	if (g_blankTex1px.glID <= 0)
	{
		u8 const whitepx[] = {0xff, 0xff, 0xff};
		g_blankTex1px = gfx::genTexture("blankTex", whitepx, TexType::Diffuse, sizeof(whitepx), 1, 1, false);
		gfx::g_blankTexID = g_blankTex1px.glID;
	}
	if (g_noTex1px.glID <= 0)
	{
		u8 const nopx[] = {0x00, 0x00, 0x00, 0x00};
		g_noTex1px = gfx::genTexture("noTex", nopx, TexType::Diffuse, sizeof(nopx), 1, 1, false);
		gfx::g_noTexID = g_noTex1px.glID;
	}
	ASSERT(g_textureMap.find(id) == g_textureMap.end(), "Texture already loaded!");
	HTexture texture = gfx::genTexture(id, std::move(bytes), type, bClampToEdge);
	if (texture.glID > 0)
	{
		g_textureMap.emplace(id, std::move(texture));
		return g_textureMap[id];
	}
	ASSERT(false, "Failed to load texture!");
	LOG_E("[Resources] [%s] Failed to load [%s]!", Typename<HTexture>().data(), id.data());
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
	LOG_W("[Resources] [%s] [%s] not found!", id.data(), Typename<HTexture>().data());
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
		gfx::releaseTexture(search->second);
		g_textureMap.erase(search);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<HTexture>()
{
	std::vector<HTexture> toDel;
	toDel.reserve(g_textureMap.size());
	for (auto& kvp : g_textureMap)
	{
		toDel.push_back(kvp.second);
	}
	gfx::releaseTextures(toDel);
	g_textureMap.clear();
}

template <>
u32 resources::count<HTexture>()
{
	return (u32)g_textureMap.size();
}

Skybox resources::createSkybox(std::string const& name, std::array<bytestream, 6> rludfb)
{
	Skybox ret;
	ret.hCube = gfx::genCubemap(name + "_map", std::move(rludfb));
	Material::Flags flags;
	flags.set(s32(Material::Flag::Textured), true);
	ret.mesh = gfx::createCube(1.0f, name + "_mesh", flags);
	ret.name = name;
	LOG_D("[%s] [%s] created", name.data(), Typename<Skybox>().data());
	return ret;
}

void resources::destroySkybox(Skybox& skybox)
{
	LOG_D("[%s] [%s] destroyed", skybox.name.data(), Typename<Skybox>().data());
	gfx::releaseCubemap(skybox.hCube);
	gfx::releaseMesh(skybox.mesh);
	skybox = Skybox();
}

BitmapFont& resources::loadFont(std::string const& id, FontAtlasData atlas)
{
	ASSERT(g_fontMap.find(id) == g_fontMap.end(), "Font already loaded!");
	BitmapFont font = gfx::newFont(id, std::move(atlas.bytes), atlas.cellSize);
	if (font.sheet.glID > 0 && font.quad.m_hVerts.vao > 0)
	{
		font.colsRows = atlas.colsRows;
		font.offset = atlas.offset;
		font.startCode = atlas.startCode;
		g_fontMap.emplace(id, std::move(font));
		return g_fontMap[id];
	}
	ASSERT(false, "Failed to load font!");
	LOG_E("[Resources] [%s] Failed to load [%s]!", Typename<BitmapFont>().data(), id.data());
	return g_nullFont;
}

template <>
BitmapFont& resources::get<BitmapFont>(std::string const& id)
{
	auto search = g_fontMap.find(id);
	if (search != g_fontMap.end())
	{
		return search->second;
	}
	LOG_W("[Resources] [%s] [%s] not found!", id.data(), Typename<BitmapFont>().data());
	return g_nullFont;
}

template <>
bool resources::isLoaded<BitmapFont>(std::string const& id)
{
	return g_fontMap.find(id) != g_fontMap.end();
}

template <>
bool resources::unload<BitmapFont>(BitmapFont& font)
{
	auto search = g_fontMap.find(font.id);
	if (search != g_fontMap.end())
	{
		gfx::releaseFont(search->second);
		g_fontMap.erase(search);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<BitmapFont>()
{
	std::vector<BitmapFont> toDel;
	toDel.reserve(g_fontMap.size());
	for (auto& kvp : g_fontMap)
	{
		gfx::releaseFont(kvp.second);
	}
	g_fontMap.clear();
}

Model& resources::loadModel(std::string const& id, Model::Data const& data)
{
	ASSERT(g_modelMap.find(id) == g_modelMap.end(), "Model already loaded!");
	Model newModel;
	newModel.setupModel(data);
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
	LOG_W("[Resources] [%s] [%s] not found!", id.data(), Typename<Model>().data());
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
	unloadAll<BitmapFont>();
	unloadAll<HTexture>();
	gfx::releaseTexture(g_blankTex1px);
	gfx::releaseTexture(g_noTex1px);
	unloadAll<HUBO>();
	unloadAll<HShader>();
}
} // namespace le
