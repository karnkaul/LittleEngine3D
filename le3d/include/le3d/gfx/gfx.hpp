#pragma once
#include <array>
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "colour.hpp"
#include "gfxtypes.hpp"
#include "mesh.hpp"
#include "bitmapFont.hpp"

namespace le::gfx
{
enum class Draw
{
	Dynamic = 0,
	Static
};

inline GLObj g_blankTexID = GLObj(1);
inline GLObj g_noTexID = GLObj(2);

HVerts genVerts(Vertices const& vertices, Draw drawType = Draw::Dynamic, HShader const* pShader = nullptr);
void releaseVerts(HVerts& outhVerts);

HTexture genTexture(std::string id, u8 const* pData, TexType type, u8 ch, u16 w, u16 h, bool bClampToEdge);
HTexture genTexture(std::string id, bytearray image, TexType type, bool bClampToEdge);
void releaseTexture(HTexture& outTexture);
void releaseTextures(std::vector<HTexture>& outTextures);

HCubemap genCubemap(std::string id, std::array<bytearray, 6> const& rludfb);
void releaseCubemap(HCubemap& outCube);

HShader genShader(std::string id, std::string_view vertCode, std::string_view fragCode);
void releaseShader(HShader& outShader);

HUBO genUBO(std::string id, s64 size, u32 bindingPoint, Draw type);
void releaseUBO(HUBO& outhUBO);

Mesh newMesh(std::string id, Vertices const& vertices, Draw type, Material::Flags flags, HShader const* pShader = nullptr);
void releaseMesh(Mesh& outMesh);

BitmapFont newFont(std::string id, bytearray spritesheet, glm::ivec2 cellSize);
void releaseFont(BitmapFont& outFont);

void setUBO(HUBO const& hUBO, s64 offset, s64 size, void const* pData);
template <typename T>
void setUBO(HUBO const& hUBO, T const& data);

// Returns true if shader tint changed (missing texture)
bool setTextures(HShader const& shader, std::vector<HTexture> const& textures, bool bSkipIfEmpty = false);
void setBlankTex(HShader const& shader, s32 txID, bool bMagenta);
// Pass -1 to reset all textures
void unsetTextures(s32 lastTexID);
void setMaterial(HShader const& shader, Material const& material);

void draw(HVerts const& hVerts);
void drawMesh(Mesh const& mesh, HShader const& shader);
void drawMeshes(Mesh const& mesh, std::vector<ModelMats> const& mats, HShader const& shader);

template <typename T>
void setUBO(HUBO const& hUBO, T const& data)
{
	setUBO(hUBO, 0, sizeof(data), &data);
}

namespace tutorial
{
HVerts newLight(HVerts const& hVBO);
}
} // namespace le::gfx
