#include "le3d/core/assert.hpp"
#include "le3d/engine/gfx/text2d.hpp"
#include "le3d/engine/gfx/gfx_store.hpp"
#include "le3d/engine/gfx/gfx_thread.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "le3d/env/env.hpp"

namespace le::gfx
{
GFXID Text2D::s_nextID = 0;

Text2D::Text2D() = default;

Text2D::Text2D(Descriptor descriptor)
{
	setup(std::move(descriptor));
}

Text2D::Text2D(Text2D&&) = default;
Text2D& Text2D::operator=(Text2D&&) = default;
Text2D::~Text2D() = default;

bool Text2D::setup(Descriptor descriptor)
{
	if (!preSetup())
	{
		return false;
	}
	m_pFont = descriptor.pFont;
	m_pShader = descriptor.pShader;
	if (!m_pFont)
	{
		m_pFont = GFXStore::instance()->get<Font>("fonts/default");
	}
	if (!m_pShader)
	{
		m_pShader = GFXStore::instance()->get<Shader>("shaders/monolithic");
	}
	ASSERT(m_pFont && m_pFont->isReady(), "Font is null!");
	ASSERT(m_pShader, "Shader is null!");
	if (!m_pFont || !m_pFont->isReady())
	{
		return false;
	}
	m_data = std::move(descriptor.data);
	VertexArray::Descriptor vertsDesc;
	vertsDesc.id = descriptor.id;
	vertsDesc.id += "_verts";
	vertsDesc.drawType = DrawType::Dynamic;
	if (m_verts.setup(std::move(vertsDesc), m_pFont->generate(m_data)))
	{
		gfx::enqueue([this]() { m_glID = ++s_nextID.handle; });
		init(std::move(descriptor.id));
		return true;
	}
	return false;
}

void Text2D::update(Font::Text data)
{
	if (isReady() && m_verts.isReady() && m_pFont && m_pFont->isReady())
	{
		m_data = std::move(data);
		m_verts.updateGeometry(m_pFont->generate(m_data));
	}
	return;
}

void Text2D::updateText(std::string text)
{
	if (isReady() && m_verts.isReady() && m_pFont && m_pFont->isReady())
	{
		if (text != m_data.text)
		{
			m_data.text = std::move(text);
			update(std::move(m_data));
		}
	}
	return;
}

VertexArray const& Text2D::vertices() const
{
	return m_verts;
}

void Text2D::render(f32 viewAspect) const
{
	if (isReady() && m_pFont && m_pFont->isReady() && m_pShader && m_pShader->isReady() && m_verts.isReady())
	{
		auto const& view = gfx::view();
		gfx::setViewport(gfx::cropView(view, viewAspect));
		auto const& u = env::g_config.uniforms;
		std::string matID;
		matID.reserve(64);
		matID += u.material.diffuseTexPrefix;
		m_pFont->m_material.tint = m_data.colour;
		m_pShader->setMaterial(m_pFont->m_material);
		m_pShader->bind({&m_pFont->m_sheet});
		m_pShader->setS32(matID, 0);
		m_pShader->setBool(u.transform.isUI, true);
		m_pShader->setBool(u.transform.isInstanced, false);
		m_pShader->setModelMats({});
		m_verts.draw(*m_pShader);
		gfx::setViewport(view);
	}
	return;
}
} // namespace le::gfx
