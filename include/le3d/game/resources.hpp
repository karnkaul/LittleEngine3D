#pragma once
#include <string>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/shading.hpp"
#include "le3d/gfx/model.hpp"

namespace le
{
namespace resources
{
HShader& loadShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags);
HShader& findShader(const std::string& id);

bool isShaderLoaded(const std::string& id);
bool unload(HShader& shader);
void unloadShaders();
u32 shaderCount();
void shadeLights(const std::vector<DirLight>& dirLights, const std::vector<PtLight>& ptLights);

extern HTexture g_blankTex1px;

HTexture& loadTexture(std::string id, std::string type, std::vector<u8> bytes);
HTexture& getTexture(const std::string& id);

bool isTextureLoaded(const std::string& id);
bool unload(HTexture& texture);
void unloadTextures(bool bUnloadBlankTex);
u32 textureCount();

HMesh& debugCube();
HMesh& debugQuad();
HMesh& debugPyramid();
HMesh& debugTetrahedron();

HMesh& debugCone();
HMesh& debugCylinder();

Model& debugArrow(const glm::quat& orientation);

void unloadAll();
} // namespace resources
} // namespace le
