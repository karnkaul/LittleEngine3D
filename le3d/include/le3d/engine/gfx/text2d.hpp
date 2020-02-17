#pragma once
#include "gfx_objects.hpp"

namespace le::gfx
{
class Text2D : public GFXObject
{
public:
	struct Descriptor
	{
		stdfs::path id;
		Font::Text data;
		Font* pFont = nullptr;
		Shader* pShader = nullptr;
	};

private:
	static GFXID s_nextID;

protected:
	Font::Text m_data;
	Font* m_pFont = nullptr;
	Shader* m_pShader = nullptr;

protected:
	VertexArray m_verts;

public:
	Text2D();
	explicit Text2D(Descriptor descriptor);
	Text2D(Text2D&&);
	Text2D& operator=(Text2D&&);
	~Text2D() override;

public:
	bool setup(Descriptor descriptor);

	void update(Font::Text data);
	void updateText(std::string text);

	VertexArray const& vertices() const;

public:
	void render(f32 viewAspect);
};
} // namespace le::gfx
