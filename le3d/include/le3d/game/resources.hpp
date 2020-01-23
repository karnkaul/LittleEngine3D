#pragma once
#include <string>
#include <unordered_map>
#include "le3d/stdtypes.hpp"
#include "le3d/core/gdata.hpp"
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
namespace jsonKeys
{
inline std::string g_samplers = "samplers";
inline std::string g_samplerID = "id";
inline std::string g_samplerWrap = "wrap";
inline std::string g_minFilter = "minFilter";
inline std::string g_magFilter = "magFilter";

inline std::string g_shaders = "shaders";
inline std::string g_shaderID = "id";
inline std::string g_vertCodeID = "vertCodeID";
inline std::string g_fragCodeID = "fragCodeID";

inline std::string g_fonts = "fonts";
inline std::string g_fontID = "id";
inline std::string g_fontJSONid = "fontID";
inline std::string g_fontTextureID = "textureID";
} // namespace jsonKeys

inline HTexture g_blankTex1px;
inline HTexture g_noTex1px;

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
u32 loadShaders(GData const& shaderList, IOReader const& reader);

HSampler& addSampler(std::string const& id, TexWrap wrap, TexFilter minFilter, TexFilter magFilter = TexFilter::Linear);
void addSamplers(GData const& samplerList);

HTexture& loadTexture(std::string const& id, TexType type, bytearray bytes);

BitmapFont& loadFont(std::string const& id, FontAtlasData atlas);
void loadFonts(GData const& fontList, IOReader const& reader);

Model& loadModel(std::string const& id, Model::Data const& data);

Skybox createSkybox(std::string const& name, std::array<bytearray, 6> rltbfb);
void destroySkybox(Skybox& skybox);

void unloadAll();
} // namespace resources
} // namespace le
