#include <array>
#include <cstddef>
#include <sstream>
#include <unordered_map>
#include <glm/gtc/type_ptr.hpp>
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/engine/context.hpp"
#include "le3d/engine/gfx/le3dgl.hpp"
#include "le3d/env/env.hpp"
#include "le3d/engine/gfx/draw.hpp"
#include "le3d/engine/gfx/gfxThread.hpp"
#include "le3d/engine/gfx/utils.hpp"
#include "engine/contextImpl.hpp"

namespace le
{
namespace
{
s32 g_maxTexUnit = 0;

//
std::unordered_map<s32, std::pair<GLObj, GLObj>> g_texUnitSamplerMap;

void setTexture(s32 unit, GLObj const& samplerID, GLObj const& textureID)
{
	auto const& pair = g_texUnitSamplerMap[unit];
	if (pair.first != samplerID || pair.second != textureID)
	{
		g_texUnitSamplerMap[unit] = {samplerID, textureID};
		auto f = gfx::enqueue([unit, sID = samplerID, tID = textureID]() {
			glChk(glActiveTexture(GL_TEXTURE0 + (GLuint)unit));
			glChk(glBindSampler((GLuint)unit, sID));
			glChk(glBindTexture(GL_TEXTURE_2D, tID));
		});
	}
	return;
}
} // namespace


void gfx::clearFlags(ClearFlags flags, Colour colour)
{
	if (context::isAlive())
	{
		gfx::enqueue([flags, colour]() {
			GLbitfield glFlags = 0;
			if (flags.isSet(ClearFlag::ColorBuffer))
			{
				glChk(glClearColor(colour.r.toF32(), colour.g.toF32(), colour.b.toF32(), colour.a.toF32()));
				glFlags |= GL_COLOR_BUFFER_BIT;
			}
			if (flags.isSet(ClearFlag::DepthBuffer))
			{
				glFlags |= GL_DEPTH_BUFFER_BIT;
			}
			if (flags.isSet(ClearFlag::StencilBuffer))
			{
				glFlags |= GL_STENCIL_BUFFER_BIT;
			}
			glClear(glFlags);
		});
	}
	return;
}

bool gfx::setTextures(HShader const& shader, std::vector<HTexture> const& textures, bool bSkipIfEmpty)
{
	bool bResetTint = false;
	if (context::isAlive())
	{
		if (bSkipIfEmpty && textures.empty())
		{
			shader.setS32(env::g_config.uniforms.material.isTextured, 0);
			return false;
		}
		shader.setS32(env::g_config.uniforms.material.isTextured, 1);
		s32 unit = 0;
		s32 diffuse = 0;
		s32 specular = 0;
		auto const& u = env::g_config.uniforms;
		if (textures.empty())
		{
			bResetTint = true;
			setBlankTex(shader, unit++, true);
		}
		bool bHasSpecular = false;
		for (auto const& texture : textures)
		{
			std::stringstream id;
			std::string number;
			number.reserve(2);
			bool bContinue = false;
			if (unit > g_maxTexUnit)
			{
				g_maxTexUnit = unit;
			}
			switch (texture.type)
			{
			case TexType::Diffuse:
			{
				id << u.material.diffuseTexPrefix;
				number = std::to_string(diffuse++);
				break;
			}
			case TexType::Specular:
			{
				bHasSpecular = true;
				id << u.material.specularTexPrefix;
				number = std::to_string(specular++);
				break;
			}
			default:
			{
				if (unit == 0)
				{
					bResetTint = true;
					setBlankTex(shader, unit, true);
				}
				bContinue = true;
				break;
			}
			}
			if (bContinue)
			{
				continue;
			}
			id << number;
			if (texture.glID > 0)
			{
				setTexture(unit, texture.hSampler.glID, texture.glID);
				shader.setS32(id.str(), unit++);
			}
			else
			{
				bResetTint = true;
				setBlankTex(shader, unit++, true);
			}
		}
		shader.setF32(u.material.hasSpecular, bHasSpecular ? 1.0f : 0.0f);
		for (; unit <= g_maxTexUnit; ++unit)
		{
			setTexture(unit, 0, 0);
		}
	}
	return bResetTint;
}

void gfx::setBlankTex(HShader const& shader, s32 textureUnit, bool bMagenta)
{
	if (context::isAlive())
	{
		if (bMagenta)
		{
			shader.setV4(env::g_config.uniforms.tint, Colour::Magenta);
		}
		setTexture(textureUnit, 0, g_blankTexID);
	}
	return;
}

void gfx::unsetTextures(s32 lastTexUnit /* = 0 */)
{
	if (context::isAlive())
	{
		if (lastTexUnit < 0)
		{
			lastTexUnit = g_maxTexUnit;
		}
		for (s32 unit = 0; unit < g_maxTexUnit && unit < lastTexUnit; ++unit)
		{
			setTexture(unit, 0, 0);
		}
	}
	return;
}

void gfx::setMaterial(HShader const& shader, Material const& material)
{
	if (context::isAlive())
	{
		auto const& u = env::g_config.uniforms;
		ASSERT(shader.glID > 0, "shader is null!");
		{
			shader.use();
			bool const bIsLit = material.flags.isSet(Material::Flag::Lit);
			shader.setBool(u.material.isLit, bIsLit);
			bool const bIsTextured = material.flags.isSet(Material::Flag::Textured);
			shader.setBool(u.material.isTextured, bIsTextured);
			if (bIsLit)
			{
				shader.setF32(u.material.shininess, material.shininess);
				shader.setV3(u.material.albedo.ambient, material.albedo.ambient);
				shader.setV3(u.material.albedo.diffuse, material.albedo.diffuse);
				shader.setV3(u.material.albedo.specular, material.albedo.specular);
			}
			if (bIsTextured)
			{
				shader.setBool(u.material.isOpaque, material.flags.isSet(Material::Flag::Opaque));
			}
		}
	}
	return;
}

void gfx::draw(HVerts const& hVerts, HShader const& shader, u32 count)
{
	if (context::isAlive())
	{
		auto f = gfx::enqueue([hVerts = hVerts, count, shader = shader]() {
			glChk(glUseProgram(shader.glID));
			glChk(glBindVertexArray(hVerts.hVAO));
			if (hVerts.hEBO > 0)
			{
				if (count > 0)
				{
					glChk(glDrawElementsInstanced(GL_TRIANGLES, hVerts.iCount, GL_UNSIGNED_INT, 0, (GLsizei)count));
				}
				else
				{
					glChk(glDrawElements(GL_TRIANGLES, hVerts.iCount, GL_UNSIGNED_INT, 0));
				}
			}
			else
			{
				if (count > 0)
				{
					glChk(glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)hVerts.vCount, (GLsizei)count));
				}
				else
				{
					glChk(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)hVerts.vCount));
				}
			}
			glChk(glBindVertexArray(0));
		});
		//gfx::wait(f);
	}
	return;
}

void gfx::drawMesh(Mesh const& mesh, HShader const& shader)
{
	setMaterial(shader, mesh.m_material);
	shader.setS32(env::g_config.uniforms.transform.isInstanced, 0);
	draw(mesh.m_hVerts, shader);
	return;
}

void gfx::drawMeshes(Mesh const& mesh, std::vector<ModelMats> const& mats, HShader const& shader)
{
	setMaterial(shader, mesh.m_material);
	shader.setS32(env::g_config.uniforms.transform.isInstanced, 0);
	for (auto const& mat : mats)
	{
		shader.setModelMats(mat);
		draw(mesh.m_hVerts, shader);
	}
	return;
}

void gfx::drawMeshes(Mesh const& mesh, HShader const& shader, u32 instanceCount)
{
	setMaterial(shader, mesh.m_material);
	shader.setS32(env::g_config.uniforms.transform.isInstanced, 1);
	draw(mesh.m_hVerts, shader, instanceCount);
	return;
}

HVerts gfx::tutorial::newLight(HVerts const& hVBO)
{
	HVerts ret;
	if (context::isAlive())
	{
		auto f = gfx::enqueue([hVBO, &ret]() {
			ret = hVBO;
			glChk(glGenVertexArrays(1, &ret.hVAO.handle));
			glChk(glBindVertexArray(ret.hVAO));
			glChk(glBindBuffer(GL_ARRAY_BUFFER, ret.hVBO.glID));
			if (ret.hEBO > 0)
			{
				glChk(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.hEBO));
			}
			glChk(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)(3 * sizeof(float)), 0));
			glChk(glEnableVertexAttribArray(0));
			glChk(glBindVertexArray(0));
		});
		if (f.valid())
		{
			f.wait();
		}
	}
	return ret;
}
} // namespace le
