#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/shading.hpp"

namespace le
{
class Mesh;

namespace resources
{
Shader& loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<Shader::MAX_FLAGS> flags);
Shader& findShader(const std::string& id);

bool isShaderLoaded(const std::string& id);
bool unload(Shader& shader);
void unloadShaders();
u32 shaderCount();

extern Texture g_blankTex1px;

Texture& loadTexture(std::string id, std::string type, std::vector<u8> bytes);
Texture& getTexture(const std::string& id);

bool isTextureLoaded(const std::string& id);
bool unload(Texture& texture);
void unloadTextures(bool bUnloadBlankTex);
u32 textureCount();

Mesh& debugMesh();
Mesh& debugQuad();

void unloadAll();
} // namespace resources
} // namespace le
