#include <glad/glad.h>
#include "le3d/core/assert.hpp"
#include "le3d/context/context.hpp"
#include "le3d/game/resources.hpp"
#include "le3d/env/env.hpp"
#include "le3d/gfx/gfx.hpp"
#include "le3d/gfx/utils.hpp"
#include "le3d/game/utils.hpp"
#include "utils.hpp"
#include "ubotypes.hpp"

namespace le
{
namespace debug
{
Text2D g_fpsStyle;
}

void debug::draw2DQuads(std::vector<Quad2D> quads, const f32 uiAR)
{
	const HShader& textured = resources::get<HShader>("ui/textured");
	const HShader& tinted = resources::get<HShader>("ui/tinted");
	const glm::vec2 vpSize = context::size();
	const glm::ivec4 glvp = viewportDxDyXY(vpSize, uiAR);
	auto& dQuad = debugQuad();
	glViewport(glvp.s, glvp.t, glvp.p, glvp.q);
	for (auto& quad : quads)
	{
		auto& quadMesh = quad.oMesh ? *quad.oMesh : dQuad;
		std::vector<HTexture> orgTex;
		std::swap(quadMesh.textures, orgTex);
		
		glm::mat4 world(1.0f);
		const HShader& shader = quad.pTexture ? textured : tinted;
		ModelMats mats;
		world = glm::translate(world, glm::vec3(quad.pos.x, quad.pos.y, 0.0f));
		mats.model = glm::scale(world, glm::vec3(quad.size.x, quad.size.y, 1.0f));
		if (quad.pTexture)
		{
			quadMesh.textures = {*quad.pTexture};
		}
		gfx::shading::setModelMats(shader, mats);
		gfx::shading::setV4(shader, env::g_config.uniforms.tint, quad.tint);
		if (quad.oTexCoords)
		{
			const glm::vec4& uv = *quad.oTexCoords;
			f32 data[] = {uv.s, uv.t, uv.p, uv.t, uv.p, uv.q, uv.p, uv.q, uv.s, uv.q, uv.s, uv.t};
			auto sf = sizeof(f32);
			glChk(glBindVertexArray(quadMesh.hVerts.vao));
			glChk(glBindBuffer(GL_ARRAY_BUFFER, quadMesh.hVerts.vbo));
			glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * 18 * 2), (GLsizeiptr)(sizeof(data)), data));
		}
		gfx::drawMesh(quadMesh, shader);
		if (quad.oTexCoords)
		{
			f32 data[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f};
			auto sf = sizeof(f32);
			glChk(glBindVertexArray(quadMesh.hVerts.vao));
			glChk(glBindBuffer(GL_ARRAY_BUFFER, quadMesh.hVerts.vbo));
			glChk(glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * 18 * 2), (GLsizeiptr)(sizeof(data)), data));
		}
		std::swap(quadMesh.textures, orgTex);
	}
	glViewport(0, 0, (s32)vpSize.x, (s32)vpSize.y);
}

void debug::renderString(const Text2D& text, const HFont& hFont, const f32 uiAR)
{
	ASSERT(hFont.sheet.glID.handle > 0, "Font has no texture!");
	const auto& shader = resources::get<HShader>("ui/textured");
	f32 cellAR = (f32)hFont.cellSize.x / hFont.cellSize.y;
	glm::vec2 cell(text.height * cellAR, text.height);
	glm::vec2 duv = {(f32)hFont.cellSize.x / hFont.sheet.size.x, (f32)hFont.cellSize.y / hFont.sheet.size.y};
	f32 width = cell.x * text.text.length();
	glm::vec2 topLeft = text.pos;
	switch (text.align)
	{
	case Align::Centre:
	{
		topLeft.x -= (width * 0.5f);
		topLeft.x += (cell.x * 0.5f);
		break;
	}
	case Align::Left:
	default:
		break;

	case Align::Right:
	{
		topLeft.x -= width;
		topLeft.x += (cell.x * 0.5f);
		break;
	}
	}
	const auto& u = env::g_config.uniforms;
	std::string matID;
	matID.reserve(128);
	matID += u.material;
	matID += ".";
	matID += u.diffuseTexPrefix;
	matID += "[0]";
	gfx::shading::setS32(shader, matID, 0);
	gfx::shading::setV4(shader, env::g_config.uniforms.tint, text.colour);
	glChk(glActiveTexture(GL_TEXTURE0));
	glChk(glBindTexture(GL_TEXTURE_2D, hFont.sheet.glID.handle));
	glBindVertexArray(hFont.quad.hVerts.vao.handle);
	glBindBuffer(GL_ARRAY_BUFFER, hFont.quad.hVerts.vbo.handle);
	const glm::vec2 vpSize = context::size();
	const glm::ivec4 glvp = viewportDxDyXY(vpSize, uiAR);
	glViewport(glvp.s, glvp.t, glvp.p, glvp.q);
	s32 idx = 0;
	for (auto c : text.text)
	{
		s32 ascii = static_cast<s32>(c);
		s32 start = (s32)hFont.startCode;
		s32 end = (s32)hFont.startCode - 1 + (s32)hFont.colsRows.x * (s32)hFont.colsRows.y;
		if (ascii >= start && ascii < end)
		{
			s32 texIdx = ascii - start;
			s32 row = hFont.colsRows.y - (texIdx / hFont.colsRows.x) - 1;
			s32 col = texIdx % hFont.colsRows.x;
			s32 y = row * hFont.cellSize.y - hFont.offset.y;
			s32 x = col * hFont.cellSize.x + hFont.offset.x;
			f32 s = (f32)x / hFont.sheet.size.x;
			f32 t = (f32)y / hFont.sheet.size.y;
			glm::vec4 uv = {s, t, s + duv.x, t + duv.y};
			glm::mat4 world(1.0f);
			glm::vec3 p = glm::vec3(topLeft.x, topLeft.y, 0.0f) + glm::vec3(cell.x * idx, 0.0f, 0.0f);
			ModelMats mats;
			world = glm::translate(world, p);
			mats.model = glm::scale(world, glm::vec3(text.height, text.height, 1.0f));
			gfx::shading::setModelMats(shader, mats);
			f32 data[] = {uv.s, uv.t, uv.p, uv.t, uv.p, uv.q, uv.p, uv.q, uv.s, uv.q, uv.s, uv.t};
			auto sf = sizeof(f32);
			glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)(sf * 18 * 2), (GLsizeiptr)(sizeof(data)), data);
			gfx::gl::draw(hFont.quad.hVerts);
		}
		++idx;
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glChk(glBindTexture(GL_TEXTURE_2D, 0));
	glViewport(0, 0, (s32)vpSize.x, (s32)vpSize.y);
}

void debug::renderFPS(const HFont& font, const f32 uiAR)
{
	static Time frameTime = Time::now();
	static Time totalDT;
	static u16 frames = 0;
	static u16 fps = 0;
	if (g_fpsStyle.pos == glm::vec2(0.0f))
	{
		g_fpsStyle.pos = {-900.0f, 500.0f};
		g_fpsStyle.height = 25.0f;
		g_fpsStyle.align = Align::Left;
		g_fpsStyle.colour = Colour(150, 150, 150);
	}
	Time dt = Time::now() - frameTime;
	totalDT += dt;
	++frames;
	if (dt > Time::secs(1.0f))
	{
		fps = frames;
		frames = 0;
		frameTime = Time::now();
	}
	g_fpsStyle.text = std::to_string(fps == 0 ? frames : fps);
	g_fpsStyle.text += " FPS";
	renderString(g_fpsStyle, font, uiAR);
}
} // namespace le
