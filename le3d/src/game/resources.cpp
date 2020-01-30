#include <memory>
#include "le3d/engine/context.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/io.hpp"
#include "le3d/core/jobs.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/tMap.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/engine/gfx/draw.hpp"
#include "le3d/engine/gfx/primitives.hpp"
#include "le3d/engine/gfx/vram.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/game/utils.hpp"

namespace le
{
namespace
{
std::unordered_map<std::string, TexWrap> g_strToTexWrap = {

	{"repeat", TexWrap::Repeat}, {"clampedge", TexWrap::ClampEdge}, {"clampborder", TexWrap::ClampBorder}

};

std::unordered_map<std::string, TexFilter> g_strToTexFilter = {

	{"linear", TexFilter::Linear},
	{"nearest", TexFilter::Nearest},
	{"linearmplinear", TexFilter::LinearMpLinear},
	{"linearmpnearest", TexFilter::LinearMpNearest},
	{"nearestmplinear", TexFilter::NearestMpLinear},
	{"nearestmpnearest", TexFilter::NearestMpNearest}

};

template <typename Resource>
using ResourceMap = TMap<std::unordered_map<std::string, Resource>>;

ResourceMap<HUBO> g_ubos;
ResourceMap<HShader> g_shaders;
ResourceMap<HSampler> g_samplers;
ResourceMap<HTexture> g_textures;
ResourceMap<BitmapFont> g_fonts;
ResourceMap<Model> g_models;
} // namespace

HUBO& resources::addUBO(std::string const& id, s64 size, u32 bindingPoint, DrawType type)
{
	ASSERT(!g_ubos.isLoaded(id), "UBO ID already loaded");
	HUBO hUBO = gfx::genUBO(id, size, bindingPoint, type);
	if (hUBO.glID > 0)
	{
		g_ubos.getMap().emplace(id, std::move(hUBO));
		return g_ubos.getMap()[id];
	}
	return g_ubos.getNull();
}

template <>
HUBO const& resources::get<HUBO>(std::string const& id)
{
	return g_ubos.get(id);
}

template <>
bool resources::isLoaded<HUBO>(std::string const& id)
{
	return g_ubos.isLoaded(id);
}

template <>
bool resources::unload<HUBO>(HUBO& hUBO)
{
	if (g_ubos.unload(hUBO.id))
	{
		gfx::releaseUBO(hUBO);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<HUBO>()
{
	for (auto& kvp : g_ubos.getMap())
	{
		gfx::releaseUBO(kvp.second);
	}
	g_ubos.unloadAll();
}

template <>
u32 resources::count<HUBO>()
{
	return g_ubos.count();
}

HShader& resources::loadShader(std::string const& id, std::string_view vertCode, std::string_view fragCode)
{
	ASSERT(!g_shaders.isLoaded(id), "Shader ID already loaded!");
	HShader shader = gfx::genShader(id, vertCode, fragCode);
	if (shader.glID > 0)
	{
		for (auto const& kvp : g_ubos.getMap())
		{
			shader.bindUBO(kvp.first, kvp.second);
		}
		g_shaders.getMap().emplace(id, std::move(shader));
		return g_shaders.getMap()[id];
	}
	ASSERT(false, "Failed to load shader!");
	LOG_E("[Resources] [%s] Failed to load [%s]!", typeName<HShader>().data(), id.data());
	return g_shaders.getNull();
}

u32 resources::loadShaders(GData const& shaderList, IOReader const& reader)
{
	auto const& r = env::g_config.jsonIDs.resources;
	if (shaderList.fieldCount() == 0)
	{
		return 0;
	}
	std::unordered_map<std::string, std::string> vertShaders;
	std::unordered_map<std::string, std::string> fragShaders;
#if defined(DEBUG_LOG)
	u32 vertsLoaded = 0;
	u32 fragsLoaded = 0;
#endif
	auto shadersData = shaderList.getGDatas(r.shaders);
	for (auto const& shaderData : shadersData)
	{
		auto const& vertShaderID = shaderData.get<std::string>(r.vertCodeID);
		auto const& fragShaderID = shaderData.get<std::string>(r.fragCodeID);
		if (vertShaderID.empty() || fragShaderID.empty() || !reader.checkPresence(vertShaderID) || !reader.checkPresence(fragShaderID))
		{
			continue;
		}
		if (vertShaders.find(vertShaderID) == vertShaders.end())
		{
			vertShaders[vertShaderID] = reader.getString(vertShaderID);
#if defined(DEBUG_LOG)
			++vertsLoaded;
#endif
		}
		if (fragShaders.find(fragShaderID) == fragShaders.end())
		{
			fragShaders[fragShaderID] = reader.getString(fragShaderID);
#if defined(DEBUG_LOG)
			++fragsLoaded;
#endif
		}
	}
	LOG_D("[Resources] [%u] Vertex and [%u] Fragment shader(s) code loaded", vertsLoaded, fragsLoaded);
	u32 processed = 0;
	for (auto const& shaderData : shadersData)
	{
		auto const& shaderID = shaderData.get<std::string>(r.shaderID);
		auto const& vertShaderID = shaderData.get<std::string>(r.vertCodeID);
		auto const& fragShaderID = shaderData.get<std::string>(r.fragCodeID);
		if (!vertShaders[vertShaderID].empty() && !fragShaders[fragShaderID].empty())
		{
			loadShader(shaderID, vertShaders[vertShaderID], fragShaders[fragShaderID]);
			++processed;
		}
		else
		{
			LOG_W("[Resources] [%s] Failed to load [%s]!", typeName<HShader>().data(), shaderID.data());
		}
	}
	LOG_D("[Resources] [%u] [%s]s loaded", processed, typeName<HShader>().data());
	return processed;
}

template <>
HShader const& resources::get<HShader>(std::string const& id)
{
	ASSERT(g_shaders.isLoaded(id), "Shader not loaded!");
	return g_shaders.get(id);
}

template <>
bool resources::isLoaded<HShader>(std::string const& id)
{
	return g_shaders.isLoaded(id);
}

template <>
bool resources::unload<HShader>(HShader& shader)
{
	if (g_shaders.unload(shader.id))
	{
		gfx::releaseShader(shader);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<HShader>()
{
	for (auto& kvp : g_shaders.getMap())
	{
		gfx::releaseShader(kvp.second);
	}
	g_shaders.unloadAll();
}

template <>
u32 resources::count<HShader>()
{
	return g_shaders.count();
}

HSampler& resources::addSampler(std::string const& id, descriptors::Sampler const& desc)
{
	ASSERT(!g_samplers.isLoaded(id), "Sampler already added!");
	HSampler hSampler = gfx::genSampler(id, desc);
	if (hSampler.glID > 0)
	{
		g_samplers.getMap()[id] = std::move(hSampler);
		return g_samplers.getMap()[id];
	}
	LOG_W("[Resources] [%s] Failed to add [%s]!", typeName<HSampler>().data(), id.data());
	return g_samplers.getNull();
}

void resources::addSamplers(GData const& samplerList)
{
	auto const& r = env::g_config.jsonIDs.resources;
	auto samplers = samplerList.getGDatas(r.samplers);
	for (auto const& samplerData : samplers)
	{
		auto id = samplerData.get<std::string>(r.samplerID);
		if (id.empty())
		{
			continue;
		}
		auto wrapStr = samplerData.get<std::string>(r.samplerWrap, "repeat");
		auto minFilterStr = samplerData.get<std::string>(r.minFilter, "linearmplinear");
		auto magFilterStr = samplerData.get<std::string>(r.magFilter, "linear");
		auto anisotropy = samplerData.get<s32>(r.anisotropy, 4);
		utils::strings::toLower(wrapStr);
		utils::strings::toLower(minFilterStr);
		utils::strings::toLower(magFilterStr);
		auto wrap = g_strToTexWrap[wrapStr];
		auto minFilter = g_strToTexFilter[minFilterStr];
		auto magFilter = g_strToTexFilter[magFilterStr];
		addSampler(id, {wrap, minFilter, magFilter, (u8)anisotropy});
	}
}

template <>
HSampler const& resources::get<HSampler>(std::string const& id)
{
	ASSERT(g_samplers.isLoaded(id), "Sampler not loaded!");
	return g_samplers.get(id);
}

template <>
bool resources::isLoaded<HSampler>(std::string const& id)
{
	return g_samplers.isLoaded(id);
}

template <>
bool resources::unload<HSampler>(HSampler& sampler)
{
	if (g_samplers.unload(sampler.id))
	{
		gfx::releaseSampler(sampler);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<HSampler>()
{
	for (auto& kvp : g_samplers.getMap())
	{
		gfx::releaseSampler(kvp.second);
	}
	g_samplers.unloadAll();
}

template <>
u32 resources::count<HSampler>()
{
	return g_samplers.count();
}

HTexture& resources::loadTexture(std::string const& id, TexType type, bytearray bytes)
{
	if (g_blankTex1px.glID <= 0)
	{
		u8 const whitepx[] = {0xff, 0xff, 0xff};
		g_blankTex1px = gfx::genTexture("blankTex", whitepx, TexType::Diffuse, nullptr, sizeof(whitepx), 1, 1);
		gfx::g_blankTexID = g_blankTex1px.glID;
		g_textures.getNull() = g_blankTex1px;
	}
	if (g_noTex1px.glID <= 0)
	{
		u8 const nopx[] = {0x00, 0x00, 0x00, 0x00};
		g_noTex1px = gfx::genTexture("noTex", nopx, TexType::Diffuse, nullptr, sizeof(nopx), 1, 1);
		gfx::g_noTexID = g_noTex1px.glID;
	}
	ASSERT(!g_textures.isLoaded(id), "Texture already loaded!");
	HTexture texture = gfx::genTexture(id, std::move(bytes), type);
	if (texture.glID > 0)
	{
		g_textures.getMap().emplace(id, std::move(texture));
		return g_textures.getMap()[id];
	}
	ASSERT(false, "Failed to load texture!");
	LOG_E("[Resources] [%s] Failed to load [%s]!", typeName<HTexture>().data(), id.data());
	return g_textures.getNull();
}

template <>
HTexture const& resources::get<HTexture>(std::string const& id)
{
	ASSERT(g_textures.isLoaded(id), "Texture not loaded!");
	return g_textures.get(id);
}

template <>
bool resources::isLoaded<HTexture>(std::string const& id)
{
	return g_textures.isLoaded(id);
}

template <>
bool resources::unload<HTexture>(HTexture& texture)
{
	if (g_textures.unload(texture.id))
	{
		gfx::releaseTexture(texture);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<HTexture>()
{
	std::vector<HTexture> toDel;
	toDel.reserve(g_textures.getMap().size());
	for (auto& kvp : g_textures.getMap())
	{
		toDel.push_back(kvp.second);
	}
	gfx::releaseTextures(toDel);
	g_textures.unloadAll();
}

template <>
u32 resources::count<HTexture>()
{
	return g_textures.count();
}

BitmapFont& resources::loadFont(std::string const& id, FontAtlasData atlas)
{
	ASSERT(!g_fonts.isLoaded(id), "Font already loaded!");
	BitmapFont font = gfx::newFont(id, std::move(atlas.bytes), atlas.cellSize, get<HSampler>(atlas.samplerID));
	if (font.sheet.glID > 0 && font.quad.m_hVerts.hVAO > 0)
	{
		font.colsRows = atlas.colsRows;
		font.offset = atlas.offset;
		font.startCode = atlas.startCode;
		g_fonts.getMap().emplace(id, std::move(font));
		return g_fonts.getMap()[id];
	}
	ASSERT(false, "Failed to load font!");
	LOG_E("[Resources] [%s] Failed to load [%s]!", typeName<BitmapFont>().data(), id.data());
	return g_fonts.getNull();
}

void resources::loadFonts(GData const& fontList, IOReader const& reader)
{
	if (fontList.fieldCount() == 0)
	{
		return;
	}
	auto const& r = env::g_config.jsonIDs.resources;
	auto const& fontsData = fontList.getGDatas(r.fonts);
	for (auto const& fontData : fontsData)
	{
		auto const& texID = fontData.get<std::string>(r.fontTextureID);
		auto const& fontID = fontData.get<std::string>(r.fontJSONid);
		auto const& id = fontData.get<std::string>(r.fontID);
		if (id.empty() || !reader.checkPresence(texID) || !reader.checkPresence(fontID))
		{
			continue;
		}
		FontAtlasData data;
		data.bytes = reader.getBytes(texID);
		data.deserialise(reader.getString(fontID));
		loadFont(id, std::move(data));
	}
}

template <>
BitmapFont const& resources::get<BitmapFont>(std::string const& id)
{
	ASSERT(g_fonts.isLoaded(id), "Font not loaded!");
	return g_fonts.get(id);
}

template <>
bool resources::isLoaded<BitmapFont>(std::string const& id)
{
	return g_fonts.isLoaded(id);
}

template <>
bool resources::unload<BitmapFont>(BitmapFont& font)
{
	if (g_fonts.unload(font.id))
	{
		gfx::releaseFont(font);
		return true;
	}
	return false;
}

template <>
void resources::unloadAll<BitmapFont>()
{
	for (auto& kvp : g_fonts.getMap())
	{
		gfx::releaseFont(kvp.second);
	}
	g_fonts.unloadAll();
}

template <>
u32 resources::count<BitmapFont>()
{
	return g_fonts.count();
}

Model& resources::loadModel(std::string const& id, Model::Data const& data)
{
	ASSERT(!g_models.isLoaded(id), "Model already loaded!");
	Model newModel;
	newModel.setupModel(data);
	g_models.getMap().emplace(id, std::move(newModel));
	return g_models.getMap()[id];
}

template <>
Model const& resources::get<Model>(std::string const& id)
{
	ASSERT(g_models.isLoaded(id), "Model not loaded!");
	return g_models.get(id);
}

template <>
bool resources::isLoaded<Model>(std::string const& id)
{
	return g_models.isLoaded(id);
}

template <>
bool resources::unload<Model>(Model& model)
{
	return g_models.unload(model.m_id);
}

template <>
void resources::unloadAll<Model>()
{
	g_models.unloadAll();
}

template <>
u32 resources::count<Model>()
{
	return g_models.count();
}

Skybox resources::createSkybox(std::string const& id, std::array<bytearray, 6> rludfb)
{
	Skybox ret;
	ret.hCube = gfx::genCubemap(id + "_map", std::move(rludfb));
	Material::Flags flags;
	flags.set(Material::Flag::Textured, true);
	ret.mesh = gfx::createCube(1.0f, id + "_mesh", flags);
	ret.id = id;
	LOG_D("[%s] [%s] created", id.data(), typeName<Skybox>().data());
	return ret;
}

void resources::destroySkybox(Skybox& skybox)
{
	LOG_D("[%s] [%s] destroyed", skybox.id.data(), typeName<Skybox>().data());
	gfx::releaseCubemap(skybox.hCube);
	gfx::releaseMesh(skybox.mesh);
	skybox = Skybox();
}

void resources::unloadAll()
{
	debug::unloadAll();
	unloadAll<Model>();
	unloadAll<BitmapFont>();
	unloadAll<HTexture>();
	gfx::releaseTexture(g_blankTex1px);
	gfx::releaseTexture(g_noTex1px);
	unloadAll<HSampler>();
	unloadAll<HUBO>();
	unloadAll<HShader>();
}
} // namespace le
