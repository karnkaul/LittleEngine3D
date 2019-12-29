#pragma once
#include <string>
#include <vector>
#include "le3d/stdtypes.hpp"

namespace le
{
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

#if defined(_WIN32) || defined(_WIN64)
constexpr char PATH_SEPARATOR = '\\';
#else
constexpr char PATH_SEPARATOR = '/';
#endif

void init(s32 argc, char** argv);
void setConfig(std::string json);
std::string_view dirPath(Dir dir);
std::string fullPath(std::string_view relative, Dir prefix);
const std::vector<std::string_view>& args();
bool isDefined(std::string_view arg);
} // namespace env
} // namespace le
