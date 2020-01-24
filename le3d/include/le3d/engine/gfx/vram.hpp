#pragma once
#include <array>
#include "le3d/core/stdtypes.hpp"
#include "gfxtypes.hpp"
#include "mesh.hpp"
#include "bitmapFont.hpp"

namespace le::gfx
{
struct VBODescriptor
{
	// Number of attributes (instances)
	u32 attribCount = 1;
	u16 attribLocation = 10;
	// Vertex attribute divisor: 1 for regular instancing
	u16 attribDivisor = 1;
	// Units of 16 bytes (each attribute is allocated as a glm::vec4)
	u8 vec4sPerAttrib = 1;
	DrawType type = DrawType::Static;
	bool bNormalised = false;
};

// Generate a VBO and activate its attribLocation for each passed hVAO
HVBO genVec4VBO(VBODescriptor const& descriptor, std::vector<GLObj> const& hVAOs);
// pData size must match hVBO.size!
void setVBO(HVBO const& hVBO, void const* pData);
void releaseVBO(HVBO& outVBO, std::vector<GLObj> const& hVAOs);

// Generate VAO, VBO, and EBO (if indexed) and setup default vertex attributes
HVerts genVerts(Vertices const& vertices, DrawType DrawTypeType = DrawType::Dynamic, HShader const* pShader = nullptr);
void releaseVerts(HVerts& outhVerts);

// Add samplers to HTextures to override default texture parameters
HSampler genSampler(std::string id, TexWrap wrap, TexFilter minFilter, TexFilter magFilter = TexFilter::Linear);
void releaseSampler(HSampler& outhSampler);

// Add pSampler to generated HTexture if passed
HTexture genTexture(std::string id, u8 const* pData, TexType type, HSampler* pSampler, u8 ch, u16 w, u16 h);
// Add pSampler to generated HTexture if passed
HTexture genTexture(std::string id, bytearray image, TexType type, HSampler* pSampler = nullptr);
void releaseTexture(HTexture& outhTexture);
void releaseTextures(std::vector<HTexture>& outhTextures);

// bytearray order: right, left, up, down, front, back
HCubemap genCubemap(std::string id, std::array<bytearray, 6> const& rludfb);
void releaseCubemap(HCubemap& outhCube);

// "version 330 core" / "version 300 es" is auto-prepended
HShader genShader(std::string id, std::string_view vertCode, std::string_view fragCode);
void releaseShader(HShader& outhShader);

// Generate shared memory for shaders (must bind to each HShader to use with)
HUBO genUBO(std::string id, s64 size, u32 bindingPoint, DrawType type);
void setUBO(HUBO const& hUBO, s64 offset, s64 size, void const* pData);
template <typename T>
void setUBO(HUBO const& hUBO, T const& data);
void releaseUBO(HUBO& outhUBO);

// Construct Mesh using passed vertices and material flags; pShader is passed to genVerts
Mesh newMesh(std::string id, Vertices const& vertices, DrawType type, Material::Flags const& flags, HShader const* pShader = nullptr);
void releaseMesh(Mesh& outMesh);

// Construct fixed cell-size BitmapFont using passed spritesheet and sampler
BitmapFont newFont(std::string id, bytearray spritesheet, glm::ivec2 cellSize, HSampler const& hSampler);
void releaseFont(BitmapFont& outFont);

template <typename T>
void setUBO(HUBO const& hUBO, T const& data)
{
	setUBO(hUBO, 0, sizeof(data), &data);
}
} // namespace le::gfx
