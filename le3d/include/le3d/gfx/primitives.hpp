#pragma once
#include "gfxtypes.hpp"
#include "model.hpp"

namespace le::gfx
{
HMesh createQuad(f32 width, f32 height, std::string name, Material::Flags materialFlags);
HMesh createCube(f32 side, std::string name, Material::Flags materialFlags);
HMesh create4Pyramid(f32 side, std::string name, Material::Flags materialFlags);
HMesh createTetrahedron(f32 side, std::string name, Material::Flags materialFlags);

HMesh createCircle(f32 diam, s32 points, std::string name, Material::Flags materialFlags);
HMesh createCone(f32 diam, f32 height, s32 points, std::string name, Material::Flags materialFlags);
HMesh createCylinder(f32 diam, f32 height, s32 points, std::string name, Material::Flags materialFlags);

HMesh createCubedSphere(f32 diam, std::string name, s32 quadsPerSide, Material::Flags materialFlags);
} // namespace le::gfx
