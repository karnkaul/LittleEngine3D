#pragma once
#include "gfxtypes.hpp"
#include "mesh.hpp"

namespace le::gfx
{
Mesh createQuad(f32 width, f32 height, std::string name, Material::Flags materialFlags);
Mesh createCube(f32 side, std::string name, Material::Flags materialFlags);
Mesh create4Pyramid(f32 side, std::string name, Material::Flags materialFlags);
Mesh createTetrahedron(f32 side, std::string name, Material::Flags materialFlags);

Mesh createCircle(f32 diam, s32 points, std::string name, Material::Flags materialFlags);
Mesh createCone(f32 diam, f32 height, s32 points, std::string name, Material::Flags materialFlags);
Mesh createCylinder(f32 diam, f32 height, s32 points, std::string name, Material::Flags materialFlags);

Mesh createCubedSphere(f32 diam, std::string name, s32 quadsPerSide, Material::Flags materialFlags);
} // namespace le::gfx
