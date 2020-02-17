#pragma once
#include <array>
#include <string>
#include <glm/glm.hpp>
#include "le3d/core/io.hpp"
#include "le3d/engine/gfx/gfx_enums.hpp"
#include "le3d/engine/gfx/gfx_objects.hpp"

namespace le::gfx::ubo
{
constexpr u8 MAX_DIR_LIGHTS = 4;
constexpr u8 MAX_PT_LIGHTS = 4;

struct Matrices final
{
	static s32 const s_bindingPoint = 1;
	static const stdfs::path s_name;
	static const DrawType s_drawType = DrawType::Dynamic;

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 viewProj = glm::mat4(1.0f);
	glm::mat4 uiProj = glm::mat4(1.0f);
	glm::vec4 viewPos = glm::vec4(0.0f);

	void setViewPos(glm::vec3 const& pos);
};

struct Lights final
{
	struct Data
	{
		Albedo albedo;
		glm::vec3 clq = glm::vec3(0.0f);
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 direction = glm::vec3(0.0f);
		size_t idx = 0;
		bool bDirLight = false;
	};

	static s32 const s_bindingPoint = 2;
	static const stdfs::path s_name;
	static const DrawType s_drawType = DrawType::Static;

	struct Pt
	{
		glm::vec4 ambient = glm::vec4(0.0f);
		glm::vec4 diffuse = glm::vec4(0.0f);
		glm::vec4 specular = glm::vec4(0.0f);
		glm::vec4 position = glm::vec4(0.0f);
		glm::vec4 clq = glm::vec4(0.0f);
	};
	struct Dir
	{
		glm::vec4 ambient = glm::vec4(0.0f);
		glm::vec4 diffuse = glm::vec4(0.0f);
		glm::vec4 specular = glm::vec4(0.0f);
		glm::vec4 direction = glm::vec4(0.0f);
	};

	std::array<Pt, MAX_PT_LIGHTS> ptLights;
	std::array<Dir, MAX_DIR_LIGHTS> dirLights;

	void setLights(std::vector<Data> const& lights);
};
} // namespace le::gfx::ubo
