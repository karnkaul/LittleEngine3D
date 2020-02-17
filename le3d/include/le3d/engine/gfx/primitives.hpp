#pragma once
#include "gfx_objects.hpp"

namespace le::gfx
{
Geometry createQuad(f32 width, f32 height);
Geometry createCube(f32 side);
Geometry create4Pyramid(f32 side);
Geometry createTetrahedron(f32 side);

Geometry createCircle(f32 diam, s32 points);
Geometry createCone(f32 diam, f32 height, s32 points);
Geometry createCylinder(f32 diam, f32 height, s32 points);

Geometry createCubedSphere(f32 diam, s32 quadsPerSide);
} // namespace le::gfx
