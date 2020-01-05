#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "le3d/stdtypes.hpp"

namespace le
{
namespace stdfs = std::filesystem;

struct EngineConfig
{
	struct Uniforms
	{
		struct Lit
		{
			std::string shininess = "material.shininess";
		};
		struct Textured
		{
			std::string hasSpecular = "material.hasSpecular";
			std::string forceOpaque = "material.forceOpaque";
			std::string diffuseTexPrefix = "material.diffuse";
			std::string specularTexPrefix = "material.specular";
		};
		struct LitTinted
		{
			std::string diffuseColour = "material.diffuse";
			std::string specularColour = "material.specular";
			std::string ambientColour = "material.ambient";
		};
		Textured textured;
		LitTinted litTinted;
		Lit lit;
		std::string tint = "tint";
		std::string modelMatrix = "model";
		std::string normalMatrix = "normals";
	};

	Uniforms uniforms;
};

namespace env
{
extern EngineConfig g_config;
extern std::string g_EOL;

enum class Dir
{
	Working,
	Executable
};

struct Args
{
	s32 argc = 0;
	char const** argv = nullptr;
};

void init(Args const& args);
void setConfig(std::string json);
stdfs::path dirPath(Dir dir);
std::vector<std::string_view> const& args();
bool isDefined(std::string_view arg);
} // namespace env
} // namespace le
