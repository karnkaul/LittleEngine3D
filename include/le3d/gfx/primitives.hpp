#pragma once
#include "gfxtypes.hpp"
#include "model.hpp"

namespace le::gfx
{
HMesh createQuad(f32 side);
HMesh createCube(f32 side);
HMesh create4Pyramid(f32 side);
HMesh createTetrahedron(f32 side);
} // namespace le::gfx
