#pragma once
#include "gfxtypes.hpp"
#include "model.hpp"

namespace le::gfx
{
HMesh createQuad(f32 side);
HMesh createCube(f32 side);
HMesh create4Pyramid(f32 side);
HMesh createTetrahedron(f32 side);

HMesh createCircle(f32 diam, s32 points);
HMesh createCone(f32 diam, f32 height, s32 points);
HMesh createCylinder(f32 diam, f32 height, s32 points);
} // namespace le::gfx
