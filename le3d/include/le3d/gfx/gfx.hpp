#pragma once
#include <array>
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/gfxtypes.hpp"

namespace le::gfx
{
enum class Draw
{
	Dynamic = 0,
	Static
};

inline GLObj g_blankTexID = GLObj(1);
inline GLObj g_noTexID = GLObj(2);

namespace gl
{
HTexture genTexture(std::string name, u8 const* pData, TexType type, u8 ch, u16 w, u16 h, bool bClampToEdge);
HTexture genTexture(std::string name, bytestream image, TexType type, bool bClampToEdge);
void releaseTexture(std::vector<HTexture*> const& textures);

HCubemap genCubemap(std::string name, std::array<bytestream, 6> const& rludfb);
void releaseCubemap(HCubemap& cube);

HShader genShader(std::string id, std::string_view vertCode, std::string_view fragCode);
void releaseShader(HShader& shader);

HVerts genVertices(Vertices const& vertices, Draw drawType = Draw::Dynamic, HShader const* pShader = nullptr);
void releaseVerts(HVerts& hVerts);

HUBO genUBO(s64 size, u32 bindingPoint, Draw type);
void releaseUBO(HUBO& hUBO);

void setMaterial(HShader const& shader, Material const& material);
void draw(HVerts const& hVerts);
} // namespace gl

void setUBO(HUBO const& hUBO, s64 offset, s64 size, void const* pData);
template <typename T>
static void setUBO(HUBO const& hUBO, T const& data)
{
	setUBO(hUBO, 0, sizeof(data), &data);
}

HMesh newMesh(std::string name, Vertices const& vertices, Draw type, Material::Flags flags, HShader const* pShader = nullptr);
void releaseMeshes(std::vector<HMesh*> const& meshes);

// Returns true if shader tint changed (missing texture)
bool setTextures(HShader const& shader, std::vector<HTexture> const& textures, bool bSkipIfEmpty = false);
void setBlankTex(HShader const& shader, s32 txID, bool bMagenta);
// Pass -1 to reset all textures
void unsetTextures(s32 lastTexID);

void drawMesh(HMesh const& mesh, HShader const& shader);
void drawMeshes(HMesh const& mesh, std::vector<ModelMats> const& mats, HShader const& shader);

void renderMesh(HMesh const& mesh, HShader const& shader);

HFont newFont(std::string name, bytestream spritesheet, glm::ivec2 cellSize);
void releaseFonts(std::vector<HFont*> const& fonts);

namespace tutorial
{
HVerts newLight(HVerts const& hVBO);
}
} // namespace le::gfx
