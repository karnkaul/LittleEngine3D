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
		struct Transform
		{
			// int
			std::string isUI = "transform.isUI";
			std::string isInstanced = "transform.isInstanced";
		};
		struct Material
		{
			struct Albedo
			{
				// vec3
				std::string ambient = "material.albedo.ambient";
				std::string diffuse = "material.albedo.diffuse";
				std::string specular = "material.albedo.specular";
			};

			Albedo albedo;
			// int
			std::string isTextured = "material.isTextured";
			std::string isLit = "material.isLit";
			std::string isOpaque = "material.isOpaque";

			// float
			std::string hasSpecular = "material.hasSpecular";
			std::string shininess = "material.shininess";

			// sampler2D
			std::string diffuseTexPrefix = "material.diffuse";
			std::string specularTexPrefix = "material.specular";
		};
		Transform transform;
		Material material;
		std::string tint = "tint";
		std::string modelMatrix = "model";
		std::string normalMatrix = "normals";
	};

	Uniforms uniforms;
};

namespace env
{
inline EngineConfig g_config;
inline std::string g_EOL = "\n";

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
std::string argv0();
std::string dirPath(Dir dir);
std::vector<std::string_view> const& args();
bool isDefined(std::string_view arg);
} // namespace env
} // namespace le
