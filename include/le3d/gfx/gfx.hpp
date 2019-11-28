#pragma once
#include <glm/glm.hpp>
#include "le3d/stdtypes.hpp"
#include "le3d/core/flags.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/gfxtypes.hpp"

namespace le::gfx
{
namespace gl
{
enum class Draw
{
	Dynamic = 0,
	Static
};

HTexture genTex(std::string name, TexType type, std::vector<u8> bytes, bool bClampToEdge);
void releaseTex(const std::vector<HTexture*>& textures);

HShader genShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags);
void releaseShader(HShader& shader);

void releaseVerts(HVerts& hVerts);

HVerts genVertices(Vertices vertices, Draw drawType = Draw::Dynamic, const HShader* pShader = nullptr);

void draw(const HVerts& hVerts, const glm::mat4& m, const glm::mat4& nm, const RenderState& rs, const HShader& s);
void draw(const HVerts& hVerts);
} // namespace gl

HMesh newMesh(std::string name, Vertices vertices, gl::Draw type, const HShader* pShader = nullptr);
void releaseMeshes(const std::vector<HMesh*>& meshes);
void drawMesh(const HMesh& mesh, const HShader& shader);

HFont newFont(std::string name, const HTexture& spritesheet, glm::ivec2 cellSize);
void releaseFonts(const std::vector<HFont*>& fonts);

namespace tutorial
{
HVerts newLight(const HVerts& hVBO);
}
} // namespace le::gfx
