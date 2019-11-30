#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/gfx/model.hpp"

namespace le
{
struct FontAtlasData
{
	glm::ivec2 cellSize = glm::ivec2(0);
	glm::ivec2 colsRows = glm::ivec2(0);
	glm::ivec2 offset = glm::ivec2(0);
	std::vector<u8> bytes;
	u8 startCode = 32;

	void deserialise(std::string json);
};

namespace resources
{
HUBO& matricesUBO();

HShader& loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags);
HShader& getShader(const std::string& id);

bool isShaderLoaded(const std::string& id);
bool unload(HShader& shader);
void unloadShaders();
u32 shaderCount();
void shadeLights(const std::vector<DirLight>& dirLights, const std::vector<PtLight>& ptLights);

extern HTexture g_blankTex1px;

HTexture& loadTexture(std::string id, TexType type, std::vector<u8> bytes, bool bClampToEdge);
HTexture& getTexture(const std::string& id);

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
