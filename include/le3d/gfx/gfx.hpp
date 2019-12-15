#pragma once
#include <array>
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/gfxtypes.hpp"

namespace le::gfx
{
enum class Draw
{
	Dynamic = 0,
	Static
};

extern GLObj g_blankTexID;

namespace gl
{
HTexture genTexture(std::string name, TexType type, std::vector<u8> bytes, bool bClampToEdge);
void releaseTexture(const std::vector<HTexture*>& textures);

HCubemap genCubemap(std::string name, std::array<std::vector<u8>, 6> rltbfb);
void releaseCubemap(HCubemap& cube);

HShader genShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags);
void releaseShader(HShader& shader);

HVerts genVertices(Vertices vertices, Draw drawType = Draw::Dynamic, const HShader* pShader = nullptr);
void releaseVerts(HVerts& hVerts);

HUBO genUBO(s64 size, u32 bindingPoint, Draw type);
void releaseUBO(HUBO& ubo);

void draw(const HVerts& hVerts);
} // namespace gl

HMesh newMesh(std::string name, Vertices vertices, Draw type, const HShader* pShader = nullptr);
void releaseMeshes(const std::vector<HMesh*>& meshes);

// Returns true if shader tint changed (missing texture)
bool setTextures(const HShader& shader, const std::vector<HTexture>& textures);
void setBlankTex(const HShader& shader, s32 txID, bool bMagenta);
// Pass -1 to reset all textures
void unsetTextures(s32 lastTexID);

void drawMesh(const HMesh& mesh, const HShader& shader);
void drawMeshes(const HMesh& mesh, const std::vector<ModelMats>& mats, const HShader& shader);

void renderMesh(const HMesh& mesh, const HShader& shader);

HFont newFont(std::string name, std::vector<u8> spritesheet, glm::ivec2 cellSize);
void releaseFonts(const std::vector<HFont*>& fonts);

namespace tutorial
{
HVerts newLight(const HVerts& hVBO);
}
} // namespace le::gfx
