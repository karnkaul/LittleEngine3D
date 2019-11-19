#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/context/context.hpp"
#include "le3d/context/contextImpl.hpp"
#include "le3d/gfx/drawable.hpp"
#include "le3d/gfx/shader.hpp"
#include "le3d/gfx/utils.hpp"

namespace le
{
Drawable::Drawable() = default;
Drawable::Drawable(Drawable&&) = default;
Drawable& Drawable::operator=(Drawable&&) = default;

Drawable::~Drawable()
{
	LOGIF_I(m_hVerts.vao > 0, "-- [%s] %s destroyed", m_name.data(), m_type.data());
	release();
}

const HVerts& Drawable::VAO() const
{
	return m_hVerts;
}

bool Drawable::setupDrawable(std::string name, std::vector<Vertex> vertices, std::vector<u32> indices, const Shader* pShader)
{
	m_name = std::move(name);
	m_type = Typename(*this);
	if (le::context::exists())
	{
		release();
		m_hVerts = gfx::newVertices(vertices, indices, pShader);
		LOGIF_I(!m_name.empty(), "== [%s] %s set up", m_name.data(), m_type.data());
		return true;
	}
	return false;
}

void Drawable::release()
{
	gfx::gl::releaseVAO(m_hVerts);
}
} // namespace le
