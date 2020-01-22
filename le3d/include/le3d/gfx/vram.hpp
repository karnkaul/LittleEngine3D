#pragma once
#include <array>
#include "le3d/stdtypes.hpp"
#include "gfxtypes.hpp"
#include "mesh.hpp"
#include "bitmapFont.hpp"

namespace le::gfx
{
struct VBODescriptor
{
	u32 attribCount = 1;
	u16 attribLocation = 10;
	u16 attribDivisor = 1;
	u8 vec4sPerAttrib = 1;
	DrawType type = DrawType::Static;
	bool bNormalised = false;
};

HVBO genVec4VBO(VBODescriptor const& descriptor, std::vector<GLObj> const& hVAOs);
void setVBO(HVBO const& hVBO, void const* pData);
void releaseVBO(HVBO& outVBO, std::vector<GLObj> const& hVAOs);

HVerts genVerts(Vertices const& vertices, DrawType DrawTypeType = DrawType::Dynamic, HShader const* pShader = nullptr);
void releaseVerts(HVerts& outhVerts);

HSampler genSampler(std::string id, TexWrap wrap, TexFilter minFilter, TexFilter magFilter = TexFilter::Linear);
void releaseSampler(HSampler& outhSampler);

HTexture genTexture(std::string id, u8 const* pData, TexType type, HSampler* pSampler, u8 ch, u16 w, u16 h);
HTexture genTexture(std::string id, bytearray image, TexType type, HSampler* pSampler = nullptr);
void releaseTexture(HTexture& outhTexture);
void releaseTextures(std::vector<HTexture>& outhTextures);

HCubemap genCubemap(std::string id, std::array<bytearray, 6> const& rludfb);
void releaseCubemap(HCubemap& outhCube);

HShader genShader(std::string id, std::string_view vertCode, std::string_view fragCode);
void releaseShader(HShader& outhShader);

HUBO genUBO(std::string id, s64 size, u32 bindingPoint, DrawType type);
void setUBO(HUBO const& hUBO, s64 offset, s64 size, void const* pData);
template <typename T>
void setUBO(HUBO const& hUBO, T const& data);
void releaseUBO(HUBO& outhUBO);

Mesh newMesh(std::string id, Vertices const& vertices, DrawType type, Material::Flags flags, HShader const* pShader = nullptr);
void releaseMesh(Mesh& outMesh);

BitmapFont newFont(std::string id, bytearray spritesheet, glm::ivec2 cellSize, HSampler const& noRepeat);
void releaseFont(BitmapFont& outFont);

template <typename T>
void setUBO(HUBO const& hUBO, T const& data)
{
	setUBO(hUBO, 0, sizeof(data), &data);
}
} // namespace le::gfx
