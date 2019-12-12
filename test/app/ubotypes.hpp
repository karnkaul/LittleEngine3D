#pragma once
#include "le3d/thirdParty.hpp"
#include "le3d/gfx/shading.hpp"

namespace le::uboData
{
constexpr u8 MAX_DIR_LIGHTS = 4;
constexpr u8 MAX_PT_LIGHTS = 4;

struct Matrices final
{
	static const s32 bindingPoint = 10;

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::vec4 viewPos = glm::vec4(0.0f);
};

struct Lights final
{
	static const s32 bindingPoint = 11;

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
};

struct UI final
{
	static const s32 bindingPoint = 12;

	glm::mat4 projection = glm::mat4(1.0f);
};
} // namespace le::uboData