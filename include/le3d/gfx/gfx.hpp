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
Texture genTex(std::string name, std::string type, std::vector<u8> bytes);
void releaseTex(std::vector<Texture*> textures);

Shader genShader(std::string id, std::string_view vertCode, std::string_view fragCode, Flags<Shader::MAX_FLAGS> flags);
void releaseShader(Shader& shader);

HVerts genVAO(bool bEBO);
void releaseVAO(HVerts& hVerts);

void bindBuffers(HVerts& hVerts, std::vector<Vertex> vertices, std::vector<u32> indices = {});
HVerts genVertices(std::vector<Vertex> vertices, std::vector<u32> indices = {}, const Shader* pShader = nullptr);

void draw(const HVerts& hVerts, const glm::mat4& m, const glm::mat4& nm, const RenderState& rs, const Shader& s);
void draw(const HVerts& hVerts);
} // namespace gl

HVerts newVertices(std::vector<Vertex> vertices, std::vector<u32> indices = {}, const Shader* pShader = nullptr);

namespace tutorial
{
HVerts newLight(const HVerts& hVBO);
}
} // namespace le::gfx
