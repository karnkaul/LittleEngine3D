#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/model.hpp"

namespace le
{
struct FontAtlasData final
{
	glm::ivec2 cellSize = glm::ivec2(0);
	glm::ivec2 colsRows = glm::ivec2(0);
	glm::ivec2 offset = glm::ivec2(0);
	std::vector<u8> bytes;
	u8 startCode = 32;

	void deserialise(std::string json);
};

struct Skybox final
{
	std::string name;
	HCubemap cubemap;
	HMesh mesh;
};

namespace resources
{
extern HTexture g_blankTex1px;

/// Supported types:
// - HFont
// - HShader
// - HTexture
// - HUBO
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

HUBO& addUBO(std::string id, s64 size, u32 bindingPoint, gfx::Draw type);
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

HShader& loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags);
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

HTexture& loadTexture(std::string id, TexType type, std::vector<u8> bytes, bool bClampToEdge);
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

Skybox createSkybox(std::string name, std::array<std::vector<u8>, 6> rltbfb);
void destroySkybox(Skybox& skybox);

HFont& loadFont(std::string id, FontAtlasData atlas);
template <>
HFont& get<HFont>(std::string const& id);
template <>
bool isLoaded<HFont>(std::string const& id);
template <>
bool unload<HFont>(HFont& font);
template <>
void unloadAll<HFont>();
template <>
u32 count<HFont>();

Model& loadModel(std::string id, Model::Data const& data, bool bForceOpaque);
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
