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
HUBO& addUBO(std::string id, s64 size, u32 bindingPoint, gfx::Draw type);
HUBO& getUBO(const std::string& id);

HShader& loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags);
HShader& getShader(const std::string& id);

bool isShaderLoaded(const std::string& id);
bool unload(HShader& shader);
void unloadShaders();
u32 shaderCount();

extern HTexture g_blankTex1px;

HTexture& loadTexture(std::string id, TexType type, std::vector<u8> bytes, bool bClampToEdge);
HTexture& getTexture(const std::string& id);

Skybox createSkybox(std::string name, std::array<std::vector<u8>, 6> rltbfb);
void destroySkybox(Skybox& skybox);

bool isTextureLoaded(const std::string& id);
bool unload(HTexture& texture);
void unloadTextures(bool bUnloadBlankTex);
u32 textureCount();

HFont& loadFont(std::string id, FontAtlasData atlas);
HFont& getFont(const std::string& id);

bool isFontLoaded(const std::string& id);
bool unload(HFont& font);
void unloadFonts();
u32 fontCount();

void unloadAll();
} // namespace resources
} // namespace le
