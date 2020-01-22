#pragma once
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "colour.hpp"
#include "gfxtypes.hpp"
#include "mesh.hpp"
#include "bitmapFont.hpp"

namespace le::gfx
{
inline GLObj g_blankTexID = GLObj(1);
inline GLObj g_noTexID = GLObj(2);

// Returns true if shader tint changed (missing texture)
bool setTextures(HShader const& shader, std::vector<HTexture> const& textures, bool bSkipIfEmpty = false);
void setBlankTex(HShader const& shader, s32 txID, bool bMagenta);
// Pass -1 to reset all textures
void unsetTextures(s32 lastTexID);
void setMaterial(HShader const& shader, Material const& material);

void draw(HVerts const& hVerts, u32 instanceCount = 0);
void drawMesh(Mesh const& mesh, HShader const& shader);
void drawMeshes(Mesh const& mesh, std::vector<ModelMats> const& mats, HShader const& shader);
void drawMeshes(Mesh const& mesh, HShader const& shader, u32 instanceCount);

namespace tutorial
{
HVerts newLight(HVerts const& hVBO);
}
} // namespace le::gfx
