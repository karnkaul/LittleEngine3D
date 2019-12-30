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
		std::string material = "material";
		std::string shininess = "material.shininess";
		std::string tint = "tint";
		std::string diffuseTexPrefix = "diffuse";
		std::string specularTexPrefix = "specular";
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

void init(s32 argc, char** argv);
void setConfig(std::string json);
stdfs::path dirPath(Dir dir);
const std::vector<std::string_view>& args();
bool isDefined(std::string_view arg);
} // namespace env
} // namespace le
