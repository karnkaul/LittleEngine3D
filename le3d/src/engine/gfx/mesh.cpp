#include "le3d/engine/gfx/mesh.hpp"

namespace le
{
Mesh::Mesh() = default;
Mesh::Mesh(Mesh&&) = default;
Mesh& Mesh::operator=(Mesh&&) = default;
Mesh::Mesh(Mesh const&) = default;
Mesh& Mesh::operator=(Mesh const&) = default;
Mesh::~Mesh() = default;
} // namespace le
