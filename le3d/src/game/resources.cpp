#include <memory>
#include "le3d/engine/context.hpp"
#include "le3d/core/assert.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/core/io.hpp"
#include "le3d/core/jobs.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/utils.hpp"
#include "le3d/gfx/draw.hpp"
#include "le3d/gfx/primitives.hpp"
#include "le3d/gfx/vram.hpp"
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

template <typename T>
class ResourceMap
{
public:
	T nullT;
	std::unordered_map<std::string, T> map;

public:
	T& get(std::string const& id);
	bool isLoaded(std::string const& id) const;
	bool unload(std::string const& id);
	void unloadAll();
	u32 count() const;
};

template <typename T>
T& ResourceMap<T>::get(std::string const& id)
{
	auto search = map.find(id);
	if (search != map.end())
	{
		return search->second;
	}
	LOG_W("[Resources] [%s] [%s] not found!", id.data(), Typename<T>().data());
	return nullT;
}

template <typename T>
bool ResourceMap<T>::isLoaded(std::string const& id) const
{
	return map.find(id) != map.end();
}

template <typename T>
bool ResourceMap<T>::unload(std::string const& id)
{
	auto search = map.find(id);
	if (search != map.end())
	{
		map.erase(search);
		return true;
	}
	return false;
}

template <typename T>
void ResourceMap<T>::unloadAll()
{
	map.clear();
}

template <typename T>
u32 ResourceMap<T>::count() const
{
	return (u32)map.size();
}

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
		g_ubos.map.emplace(id, std::move(hUBO));
		return g_ubos.map[id];
	}
	return g_ubos.nullT;
}

template <>
HUBO& resources::get<HUBO>(std::string const& id)
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
	for (auto& kvp : g_ubos.map)
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
	if (shader.glID.handle > 0)
	{
		for (auto const& kvp : g_ubos.map)
		{
			shader.bindUBO(kvp.first, kvp.second);
		}
		g_shaders.map.emplace(id, std::move(shader));
		return g_shaders.map[id];
	}
	ASSERT(false, "Failed to load shader!");
	LOG_E("[Resources] [%s] Failed to load [%s]!", Typename<HShader>().data(), id.data());
	return g_shaders.nullT;
}

u32 resources::loadShaders(ShaderIDMap const& data, IOReader const& reader)
{
	if (data.empty())
	{
		return 0;
	}
	std::unordered_map<std::string, std::string> vertShaders;
	std::unordered_map<std::string, std::string> fragShaders;
#if defined(DEBUG_LOG)
	u32 vertsLoaded = 0;
	u32 fragsLoaded = 0;
#endif
	for (auto const& kvp : data)
	{
		auto const& vertShaderID = kvp.second.first;
		auto const& fragShaderID = kvp.second.second;
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
	for (auto const& kvp : data)
	{
		auto const& vertShaderID = kvp.second.first;
		auto const& fragShaderID = kvp.second.second;
		if (!vertShaders[vertShaderID].empty() && !fragShaders[fragShaderID].empty())
		{
			loadShader(kvp.first, vertShaders[vertShaderID], fragShaders[fragShaderID]);
			++processed;
		}
		else
		{
			LOG_W("[Resources] [%s] Failed to load [%s]!", Typename<HShader>().data(), kvp.first.data());
		}
	}
	LOG_D("[Resources] [%u] [%s]s loaded", processed, Typename<HShader>().data());
	return processed;
}

template <>
HShader& resources::get<HShader>(std::string const& id)
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
	for (auto& kvp : g_shaders.map)
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

HSampler& resources::addSampler(std::string const& id, TexWrap wrap, TexFilter minfilter, TexFilter magFilter)
{
	ASSERT(!g_samplers.isLoaded(id), "Sampler already added!");
	HSampler hSampler = gfx::genSampler(id, wrap, minfilter, magFilter);
	if (hSampler.glID > 0)
	{
		g_samplers.map[id] = std::move(hSampler);
		return g_samplers.map[id];
	}
	LOG_W("[Resources] [%s] Failed to add [%s]!", Typename<HSampler>().data(), id.data());
	return g_samplers.nullT;
}

void resources::addSamplers(std::string json)
{
	GData gData(std::move(json));
	auto samplerList = gData.getGDatas("samplers");
	for (auto const& samplerData : samplerList)
	{
		auto id = samplerData.getStr("id");
		if (id.empty())
		{
			continue;
		}
		auto wrapStr = samplerData.getStr("wrap", "repeat");
		auto minFilterStr = samplerData.getStr("minFilter", "linearmplinear");
		auto magFilterStr = samplerData.getStr("magFilter", "linear");
		utils::strings::toLower(wrapStr);
		utils::strings::toLower(minFilterStr);
		utils::strings::toLower(magFilterStr);
		auto wrap = g_strToTexWrap[wrapStr];
		auto minFilter = g_strToTexFilter[minFilterStr];
		auto magFilter = g_strToTexFilter[magFilterStr];
		addSampler(id, wrap, minFilter, magFilter);
	}
}

template <>
HSampler& resources::get<HSampler>(std::string const& id)
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
	for (auto& kvp : g_samplers.map)
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
		g_textures.nullT = g_blankTex1px;
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
		g_textures.map.emplace(id, std::move(texture));
		return g_textures.map[id];
	}
	ASSERT(false, "Failed to load texture!");
	LOG_E("[Resources] [%s] Failed to load [%s]!", Typename<HTexture>().data(), id.data());
	return g_textures.nullT;
}

template <>
HTexture& resources::get<HTexture>(std::string const& id)
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
	toDel.reserve(g_textures.map.size());
	for (auto& kvp : g_textures.map)
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
	if (g_fontSampler.glID == 0)
	{
		g_fontSampler = gfx::genSampler("clampEdge", TexWrap::ClampEdge, TexFilter::Linear);
	}
	BitmapFont font = gfx::newFont(id, std::move(atlas.bytes), atlas.cellSize, g_fontSampler);
	if (font.sheet.glID > 0 && font.quad.m_hVerts.hVAO > 0)
	{
		font.colsRows = atlas.colsRows;
		font.offset = atlas.offset;
		font.startCode = atlas.startCode;
		g_fonts.map.emplace(id, std::move(font));
		return g_fonts.map[id];
	}
	ASSERT(false, "Failed to load font!");
	LOG_E("[Resources] [%s] Failed to load [%s]!", Typename<BitmapFont>().data(), id.data());
	return g_fonts.nullT;
}

template <>
BitmapFont& resources::get<BitmapFont>(std::string const& id)
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
	for (auto& kvp : g_fonts.map)
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
	g_models.map.emplace(id, std::move(newModel));
	return g_models.map[id];
}

template <>
Model& resources::get<Model>(std::string const& id)
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

Skybox resources::createSkybox(std::string const& name, std::array<bytearray, 6> rludfb)
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

void resources::unloadAll()
{
	debug::unloadAll();
	unloadAll<Model>();
	unloadAll<BitmapFont>();
	unloadAll<HTexture>();
	gfx::releaseTexture(g_blankTex1px);
	gfx::releaseTexture(g_noTex1px);
	gfx::releaseSampler(g_fontSampler);
	unloadAll<HSampler>();
	unloadAll<HUBO>();
	unloadAll<HShader>();
}
} // namespace le
