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
#include "le3d/engine/gfx/utils.hpp"
#include "engine/contextImpl.hpp"

namespace le
{
namespace
{
s32 g_maxTexIdx = 0;

//
std::unordered_map<s32, std::pair<GLObj, GLObj>> g_texUnitSamplerMap;

void setTexture(s32 txID, GLObj const& samplerID, GLObj const& textureID)
{
	auto const& pair = g_texUnitSamplerMap[txID];
	if (pair.first != samplerID || pair.second != textureID)
	{
		g_texUnitSamplerMap[txID] = {samplerID, textureID};
		glChk(glActiveTexture(GL_TEXTURE0 + (GLuint)txID));
		glChk(glBindSampler((GLuint)txID, samplerID.handle));
		glChk(glBindTexture(GL_TEXTURE_2D, textureID.handle));
	}
}
} // namespace

bool gfx::setTextures(HShader const& shader, std::vector<HTexture> const& textures, bool bSkipIfEmpty)
{
	bool bResetTint = false;
	if (context::isAlive())
	{
		cxChk();
		if (bSkipIfEmpty && textures.empty())
		{
			shader.setS32(env::g_config.uniforms.material.isTextured, 0);
			return false;
		}
		shader.setS32(env::g_config.uniforms.material.isTextured, 1);
		s32 txID = 0;
		s32 diffuse = 0;
		s32 specular = 0;
		auto const& u = env::g_config.uniforms;
		if (textures.empty())
		{
			bResetTint = true;
			setBlankTex(shader, txID++, true);
		}
		bool bHasSpecular = false;
		for (auto const& texture : textures)
		{
			std::stringstream id;
			std::string number;
			number.reserve(2);
			bool bContinue = false;
			if (txID > g_maxTexIdx)
			{
				g_maxTexIdx = txID;
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
				if (txID == 0)
				{
					bResetTint = true;
					setBlankTex(shader, txID, true);
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
			if (texture.glID.handle > 0)
			{
				setTexture(txID, texture.hSampler.glID, texture.glID);
				shader.setS32(id.str(), txID++);
			}
			else
			{
				bResetTint = true;
				setBlankTex(shader, txID++, true);
			}
		}
		shader.setF32(u.material.hasSpecular, bHasSpecular ? 1.0f : 0.0f);
		for (; txID <= g_maxTexIdx; ++txID)
		{
			setTexture(txID, 0, 0);
		}
	}
	return bResetTint;
}

void gfx::setBlankTex(HShader const& shader, s32 txID, bool bMagenta)
{
	if (context::isAlive())
	{
		cxChk();
		if (bMagenta)
		{
			shader.setV4(env::g_config.uniforms.tint, Colour::Magenta);
		}
		setTexture(txID, 0, g_blankTexID);
	}
}

void gfx::unsetTextures(s32 lastTexID /* = 0 */)
{
	if (context::isAlive())
	{
		cxChk();
		if (lastTexID < 0)
		{
			lastTexID = g_maxTexIdx;
		}
		for (s32 txID = 0; txID < g_maxTexIdx && txID < lastTexID; ++txID)
		{
			setTexture(txID, 0, 0);
		}
	}
}

void gfx::setMaterial(HShader const& shader, Material const& material)
{
	if (context::isAlive())
	{
		auto const& u = env::g_config.uniforms;
		ASSERT(shader.glID.handle > 0, "shader is null!");
		{
			cxChk();
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
}

void gfx::draw(HVerts const& hVerts, u32 count)
{
	if (context::isAlive())
	{
		cxChk();
		glChk(glBindVertexArray(hVerts.hVAO.handle));
		if (hVerts.hEBO.handle > 0)
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
		glBindVertexArray(0);
	}
}

void gfx::drawMesh(Mesh const& mesh, HShader const& shader)
{
	cxChk();
	setMaterial(shader, mesh.m_material);
	shader.setS32(env::g_config.uniforms.transform.isInstanced, 0);
	draw(mesh.m_hVerts);
}

void gfx::drawMeshes(Mesh const& mesh, std::vector<ModelMats> const& mats, HShader const& shader)
{
	cxChk();
	setMaterial(shader, mesh.m_material);
	shader.setS32(env::g_config.uniforms.transform.isInstanced, 0);
	for (auto const& mat : mats)
	{
		shader.setModelMats(mat);
		draw(mesh.m_hVerts);
	}
}

void gfx::drawMeshes(Mesh const& mesh, HShader const& shader, u32 instanceCount)
{
	cxChk();
	setMaterial(shader, mesh.m_material);
	shader.setS32(env::g_config.uniforms.transform.isInstanced, 1);
	draw(mesh.m_hVerts, instanceCount);
}

HVerts gfx::tutorial::newLight(HVerts const& hVBO)
{
	HVerts ret;
	if (context::isAlive())
	{
		cxChk();
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
	}
	return ret;
}
} // namespace le
