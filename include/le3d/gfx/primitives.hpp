#pragma once
#include "gfxtypes.hpp"
#include "model.hpp"

namespace le::gfx
{
HMesh createQuad(f32 width, f32 height, std::string name);
HMesh createCube(f32 side, std::string name);
HMesh create4Pyramid(f32 side, std::string name);
HMesh createTetrahedron(f32 side, std::string name);

HMesh createCircle(f32 diam, s32 points, std::string name);
HMesh createCone(f32 diam, f32 height, s32 points, std::string name);
HMesh createCylinder(f32 diam, f32 height, s32 points, std::string name);

HMesh createCubedSphere(f32 diam, std::string name, s32 quadsPerSide = 8);
} // namespace le::gfx
