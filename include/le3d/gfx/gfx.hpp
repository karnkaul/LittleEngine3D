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
enum class Type
{
	Dynamic = 0,
	Static
};

HTexture genTex(std::string name, std::string type, std::vector<u8> bytes);
void releaseTex(std::vector<HTexture*> textures);

HShader genShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<HShader::MAX_FLAGS> flags);
void releaseShader(HShader& shader);

void releaseVerts(HVerts& hVerts);

HVerts genVertices(Vertices vertices, Type drawType = Type::Dynamic, const HShader* pShader = nullptr);

void draw(const HVerts& hVerts, const glm::mat4& m, const glm::mat4& nm, const RenderState& rs, const HShader& s);
void draw(const HVerts& hVerts);
} // namespace gl

HMesh newMesh(std::string name, Vertices vertices, gl::Type type, const HShader* pShader = nullptr);
void releaseMeshes(std::vector<HMesh*> mesh);
void drawMesh(const HMesh& mesh, const HShader& shader);

namespace tutorial
{
HVerts newLight(const HVerts& hVBO);
}
} // namespace le::gfx
