#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/shading.hpp"
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

template <typename T>
T& get(const std::string& id);
template <typename T>
bool isLoaded(const std::string& id);
template <typename T>
bool unload(T& t);
template <typename T>
void unloadAll();
template <typename T>
u32 count();

HUBO& addUBO(std::string id, s64 size, u32 bindingPoint, gfx::Draw type);
template <>
HUBO& get<HUBO>(const std::string& id);
template <>
bool isLoaded<HUBO>(const std::string& id);
template <>
bool unload<HUBO>(HUBO& hUBO);
template <>
void unloadAll<HUBO>();
template <>
u32 count<HUBO>();

HShader& loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags);
template <>
HShader& get<HShader>(const std::string& id);
template <>
bool isLoaded<HShader>(const std::string& id);
template <>
bool unload<HShader>(HShader& shader);
template <>
void unloadAll<HShader>();
template <>
u32 count<HShader>();

HTexture& loadTexture(std::string id, TexType type, std::vector<u8> bytes, bool bClampToEdge);
template <>
HTexture& get<HTexture>(const std::string& id);
template <>
bool isLoaded<HTexture>(const std::string& id);
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
HFont& get<HFont>(const std::string& id);
template <>
bool isLoaded<HFont>(const std::string& id);
template <>
bool unload<HFont>(HFont& font);
template <>
void unloadAll<HFont>();
template <>
u32 count<HFont>();

void unloadAll();
} // namespace resources
} // namespace le
