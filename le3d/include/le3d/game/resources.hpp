#pragma once
#include <string>
#include <unordered_map>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/bitmapFont.hpp"
#include "le3d/gfx/gfxtypes.hpp"
#include "le3d/gfx/model.hpp"

namespace le
{
struct Skybox final
{
	Mesh mesh;
	std::string name;
	HCubemap hCube;
};

class IOReader;

namespace resources
{
// {shaderID, {vertCodeID, fragCodeID}}
using ShaderIDMap = std::unordered_map<std::string, std::pair<std::string, std::string>>;

inline HTexture g_blankTex1px;
inline HTexture g_noTex1px;
inline HSampler g_fontSampler;

/// Supported types:
// - HSampler
// - HShader
// - HTexture
// - HUBO
// - BitmapFont
// - Model

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

HUBO& addUBO(std::string const& id, s64 size, u32 bindingPoint, DrawType type);

HShader& loadShader(std::string const& id, std::string_view vertCode, std::string_view fragCode);
u32 loadShaders(ShaderIDMap const& data, IOReader const& reader);

HSampler& addSampler(std::string const& id, TexWrap wrap, TexFilter minFilter, TexFilter magFilter = TexFilter::Linear);
void addSamplers(std::string json);

HTexture& loadTexture(std::string const& id, TexType type, bytearray bytes);

BitmapFont& loadFont(std::string const& id, FontAtlasData atlas);

Model& loadModel(std::string const& id, Model::Data const& data);

Skybox createSkybox(std::string const& name, std::array<bytearray, 6> rltbfb);
void destroySkybox(Skybox& skybox);

void unloadAll();
} // namespace resources
} // namespace le
