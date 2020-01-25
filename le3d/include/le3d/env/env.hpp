#pragma once
#include <string>
#include <vector>
#include "le3d/core/stdtypes.hpp"

#if (defined(_WIN32) || defined(_WIN64))
#define LE3D_OS_WINX
#if defined(__arm__)
#define LE3D_ARCH_ARM64
#define LE3D_NON_DESKTOP
#elif !defined(_WIN64)
#define LE3D_ARCH_X86
#else
#define LE3D_ARCH_x64
#endif
#elif defined(__linux__)
#if defined(__ANDROID__)
#define LE3D_OS_ANDROID
#define LE3D_NON_DESKTOP
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

	struct JSONIDs
	{
		struct Resources
		{
			std::string samplers = "samplers";
			std::string samplerID = "id";
			std::string samplerWrap = "wrap";
			std::string minFilter = "minFilter";
			std::string magFilter = "magFilter";

			std::string shaders = "shaders";
			std::string shaderID = "id";
			std::string vertCodeID = "vertCodeID";
			std::string fragCodeID = "fragCodeID";

			std::string fonts = "fonts";
			std::string fontID = "id";
			std::string fontJSONid = "fontID";
			std::string fontTextureID = "textureID";
		};
		Resources resources;
	};

	Uniforms uniforms;
	JSONIDs jsonIDs;
	std::string shaderPrefix = "#version 330 core";
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
