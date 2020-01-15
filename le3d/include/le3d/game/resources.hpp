#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/model.hpp"

namespace le
{
struct Skybox final
{
	Mesh mesh;
	std::string name;
	HCubemap hCube;
};

namespace resources
{
inline HTexture g_blankTex1px;
inline HTexture g_noTex1px;

/// Supported types:
// - HShader
// - HTexture
// - HUBO
// - Model
// - BitmapFont

template <typename T>
T& get(std::string const& id);
template <typename T>
bool isLoaded(std::string const& id);
template <typename T>
bool unload(T& t);
template <typename T>
void unloadAll();
template <typename T>
u32 count();

HUBO& addUBO(std::string const& id, s64 size, u32 bindingPoint, gfx::Draw type);
template <>
HUBO& get<HUBO>(std::string const& id);
template <>
bool isLoaded<HUBO>(std::string const& id);
template <>
bool unload<HUBO>(HUBO& hUBO);
template <>
void unloadAll<HUBO>();
template <>
u32 count<HUBO>();

HShader& loadShader(std::string const& id, std::string_view vertCode, std::string_view fragCode);
template <>
HShader& get<HShader>(std::string const& id);
template <>
bool isLoaded<HShader>(std::string const& id);
template <>
bool unload<HShader>(HShader& shader);
template <>
void unloadAll<HShader>();
template <>
u32 count<HShader>();

HTexture& loadTexture(std::string const& id, TexType type, bytestream bytes, bool bClampToEdge);
template <>
HTexture& get<HTexture>(std::string const& id);
template <>
bool isLoaded<HTexture>(std::string const& id);
template <>
bool unload<HTexture>(HTexture& texture);
template <>
void unloadAll<HTexture>();
template <>
u32 count<HTexture>();

Skybox createSkybox(std::string const& name, std::array<bytestream, 6> rltbfb);
void destroySkybox(Skybox& skybox);

BitmapFont& loadFont(std::string const& id, FontAtlasData atlas);
template <>
BitmapFont& get<BitmapFont>(std::string const& id);
template <>
bool isLoaded<BitmapFont>(std::string const& id);
template <>
bool unload<BitmapFont>(BitmapFont& font);
template <>
void unloadAll<BitmapFont>();
template <>
u32 count<BitmapFont>();

Model& loadModel(std::string const& id, Model::Data const& data);
template <>
Model& get<Model>(std::string const& id);
template <>
bool isLoaded<Model>(std::string const& id);
template <>
bool unload<Model>(Model& id);
template <>
void unloadAll<Model>();
template <>
u32 count<Model>();

void unloadAll();
} // namespace resources
} // namespace le
