#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "le3d/core/std_types.hpp"

#if (defined(_WIN32) || defined(_WIN64))
#define LE3D_OS_WINX
#if defined(__arm__)
#define LE3D_ARCH_ARM64
#elif !defined(_WIN64)
#define LE3D_ARCH_X86
#else
#define LE3D_ARCH_x64
#endif
#elif defined(__linux__)
#if defined(__ANDROID__)
#define LE3D_OS_ANDROID
#else
#define LE3D_OS_LINUX
#endif
#if defined(__arm__)
#define LE3D_ARCH_ARM64
#elif defined(__x86_64__)
#define LE3D_ARCH_X64
#elif defined(__i386__)
#define LE3D_ARCH_X86
#else
#define LE3D_ARCH_UNSUPPORTED
#endif
#else
#define LE3D_OS_UNSUPPORTED
#endif

#if defined(_MSC_VER)
#define LE3D_RUNTIME_MSVC
#elif (defined(__GNUG__) || defined(__clang__))
#define LE3D_RUNTIME_LIBSTDCXX
#else
#define LE3D_RUNTIME_UNKNOWN
#endif

namespace le
{
struct EngineConfig
{
	struct Uniforms
	{
		struct Transform
		{
			// int
			std::string_view isUI = "transform.isUI";
			std::string_view isInstanced = "transform.isInstanced";
		};
		struct Material
		{
			struct Albedo
			{
				// vec3
				std::string_view ambient = "material.albedo.ambient";
				std::string_view diffuse = "material.albedo.diffuse";
				std::string_view specular = "material.albedo.specular";
				std::string_view shininess = "material.albedo.shininess";
			};

			Albedo albedo;
			// int
			std::string_view isTextured = "material.isTextured";
			std::string_view isLit = "material.isLit";
			std::string_view isOpaque = "material.isOpaque";
			std::string_view isFont = "material.isFont";

			// float
			std::string_view hasSpecular = "material.hasSpecular";

			// sampler2D
			std::string_view diffuseTexPrefix = "material.diffuse";
			std::string_view specularTexPrefix = "material.specular";

			// vec4
			std::string_view tint = "tint";

			// samplerCube
			std::string_view skybox = "skybox";
		};
		Transform transform;
		Material material;
		std::string_view modelMatrix = "model";
		std::string_view normalMatrix = "normals";
	};

	Uniforms uniforms;
	std::string_view shaderPrefix = "#version 330 core";
};

namespace env
{
inline EngineConfig g_config;
inline std::string_view g_EOL = "\n";

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
std::string_view argv0();
std::filesystem::path dirPath(Dir dir);
std::vector<std::string_view> const& args();
bool isDefined(std::string_view arg);
} // namespace env
} // namespace le
