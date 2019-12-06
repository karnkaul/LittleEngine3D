#pragma once
#include "le3d/core/time.hpp"
#include "le3d/stdtypes.hpp"
#include "le3d/gfx/colour.hpp"
#include "le3d/gfx/gfxtypes.hpp"

namespace le
{
void renderSkybox(const struct Skybox& skybox, const HShader& shader, Colour tint = Colour::White);
void renderMeshes(const HMesh& mesh, const std::vector<glm::mat4> m, const std::vector<glm::mat4> nm, const HShader& shader,
				  Colour tint = Colour::White);

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
