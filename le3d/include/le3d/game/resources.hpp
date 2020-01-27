#pragma once
#include <string>
#include <unordered_map>
#include "le3d/core/stdtypes.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/engine/gfx/bitmapFont.hpp"
#include "le3d/engine/gfx/gfxtypes.hpp"
#include "le3d/engine/gfx/model.hpp"

namespace le
{
struct Skybox final
{
	Mesh mesh;
	std::string id;
	HCubemap hCube;
};

class IOReader;

namespace resources
{
inline HTexture g_blankTex1px;
inline HTexture g_noTex1px;

template <typename T>
T const& get(std::string const& id);
template <typename T>
bool isLoaded(std::string const& id);
template <typename T>
bool unload(T& t);
template <typename T>
void unloadAll();
template <typename T>
u32 count();

HUBO& addUBO(std::string const& id, s64 size, u32 bindingPoint, DrawType type);

HShader& loadShader(std::string const& id, std::string_view vertCode, std::string_view fragCode);
// Shader data fields: id, vertCodeID, fragCodeID
u32 loadShaders(GData const& shaderList, IOReader const& reader);

HSampler& addSampler(std::string const& id, descriptors::Sampler const& desc);
// Sampler data fields: id, wrap, minFilter, magFilter
void addSamplers(GData const& samplerList);

HTexture& loadTexture(std::string const& id, TexType type, bytearray bytes);

BitmapFont& loadFont(std::string const& id, FontAtlasData atlas);
// Font data fields: id, fontID, textureID
void loadFonts(GData const& fontList, IOReader const& reader);

Model& loadModel(std::string const& id, Model::Data const& data);

Skybox createSkybox(std::string const& id, std::array<bytearray, 6> rltbfb);
void destroySkybox(Skybox& skybox);

void unloadAll();

template <typename T>
T const& get(std::string const&)
{
	static_assert(alwaysFalse<T>, "Invalid Type!");
}
template <typename T>
bool isLoaded(std::string const&)
{
	static_assert(alwaysFalse<T>, "Invalid Type!");
}
template <typename T>
bool unload(T&)
{
	static_assert(alwaysFalse<T>, "Invalid Type!");
}
template <typename T>
void unloadAll()
{
	static_assert(alwaysFalse<T>, "Invalid Type!");
}
template <typename T>
u32 count()
{
	static_assert(alwaysFalse<T>, "Invalid Type!");
}

template <>
HUBO const& get(std::string const& id);
template <>
HShader const& get(std::string const& id);
template <>
HSampler const& get(std::string const& id);
template <>
HTexture const& get(std::string const& id);
template <>
BitmapFont const& get(std::string const& id);
template <>
Model const& get(std::string const& id);
template <>
bool isLoaded<HUBO>(std::string const& id);
template <>
bool isLoaded<HShader>(std::string const& id);
template <>
bool isLoaded<HSampler>(std::string const& id);
template <>
bool isLoaded<HTexture>(std::string const& id);
template <>
bool isLoaded<BitmapFont>(std::string const& id);
template <>
bool isLoaded<Model>(std::string const& id);
template <>
bool unload<HUBO>(HUBO& hUBO);
template <>
bool unload<HShader>(HShader& shader);
template <>
bool unload<HSampler>(HSampler& hSampler);
template <>
bool unload<HTexture>(HTexture& hTexture);
template <>
bool unload<BitmapFont>(BitmapFont& font);
template <>
bool unload<Model>(Model& model);
template <>
void unloadAll<HUBO>();
template <>
void unloadAll<HShader>();
template <>
void unloadAll<HSampler>();
template <>
void unloadAll<HTexture>();
template <>
void unloadAll<BitmapFont>();
template <>
void unloadAll<Model>();
template <>
u32 count<HUBO>();
template <>
u32 count<HShader>();
template <>
u32 count<HSampler>();
template <>
u32 count<HTexture>();
template <>
u32 count<BitmapFont>();
template <>
u32 count<Model>();
} // namespace resources
} // namespace le
