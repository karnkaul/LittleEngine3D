#pragma once
#include "le3d/core/time.hpp"
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/gfxtypes.hpp"

namespace le
{
void renderSkybox(const struct Skybox& skybox, const HShader& shader, Colour tint = Colour::White);
void renderMeshes(const HMesh& mesh, const std::vector<ModelMats>& mats, const HShader& shader, Colour tint = Colour::White);

namespace debug
{
HMesh& debugCube();
HMesh& debugQuad();
HMesh& debugPyramid();
HMesh& debugTetrahedron();

HMesh& debugCone();
HMesh& debugCylinder();

Model& debugArrow(const glm::quat& orientation);
} // namespace debug
} // namespace le
