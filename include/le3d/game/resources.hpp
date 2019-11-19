#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/shader.hpp"

namespace le
{
namespace resources
{
Shader* loadShader(std::string id, std::string_view vertCode, std::string_view fragCode);
Shader* findShader(const std::string& id);
Shader& getShader(const std::string& id);

bool unload(Shader& shader);
void unloadShaders();
u32 shaderCount();

extern Texture g_blankTex1px;

Texture* loadTexture(std::string id, std::string type, std::vector<u8> bytes);
Texture* findTexture(const std::string& id);
Texture& getTexture(const std::string& id);

bool unload(Texture& texture);
void unloadTextures(bool bUnloadBlankTex);
u32 textureCount();
} // namespace resources
} // namespace le
