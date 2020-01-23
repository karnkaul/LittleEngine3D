#include <algorithm>
#include <filesystem>
#if defined(__linux__)
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#endif
#include "le3d/core/assert.hpp"
#include "le3d/core/log.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/env/env.hpp"
#include "env/threadsImpl.hpp"

namespace le
{
namespace stdfs = std::filesystem;

namespace
{
stdfs::path g_exeLocation;
stdfs::path g_exePath;
stdfs::path g_workingDir;
std::vector<std::string_view> g_args;

void SetConfigStrIfPresent(std::string const& id, GData const& data, std::string& member)
{
	if (data.contains(id))
	{
		member = data.getString(id);
		LOG_D("[EngineConfig] Extracted [%s] = [%s]", id.data(), member.data());
	}
}

// void SetConfigS32IfPresent(std::string const& id, GData const& data, s32& member)
//{
//	if (data.contains(id))
//	{
//		member = data.getS32(id);
//		LOG_D("[EngineConfig] Extracted [%s] = [%d]", id.data(), member);
//	}
//}
} // namespace

void env::init(Args const& args)
{
#if defined(__linux__)
	s32 threadStatus = XInitThreads();
	if (threadStatus == 0)
	{
		LOG_E("[OS] ERROR calling XInitThreads()! UB follows.");
		threadsImpl::g_maxThreads = 1;
	}
#endif
	g_workingDir = std::filesystem::current_path();
	if (args.argc > 0)
	{
		g_exeLocation = args.argv[0];
		g_exePath = g_exeLocation.parent_path();
		for (s32 i = 1; i < args.argc; ++i)
		{
			g_args.push_back(args.argv[i]);
		}
	}
}

void env::setConfig(std::string json)
{
	GData data(json);
	if (data.fieldCount() > 0)
	{
		if (data.contains("uniforms"))
		{
			GData uniforms = data.getGData("uniforms");
			SetConfigStrIfPresent("modelMatrix", uniforms, g_config.uniforms.modelMatrix);
			SetConfigStrIfPresent("normalMatrix", uniforms, g_config.uniforms.normalMatrix);
			SetConfigStrIfPresent("tint", uniforms, g_config.uniforms.tint);
			if (uniforms.contains("transform"))
			{
				GData transform = uniforms.getGData("uniforms");
				SetConfigStrIfPresent("isUI", transform, g_config.uniforms.transform.isUI);
			}
			if (uniforms.contains("material"))
			{
				GData material = uniforms.getGData("material");
				SetConfigStrIfPresent("isTextured", material, g_config.uniforms.material.isTextured);
				SetConfigStrIfPresent("isLit", material, g_config.uniforms.material.isLit);
				SetConfigStrIfPresent("isOpaque", material, g_config.uniforms.material.isOpaque);
				SetConfigStrIfPresent("hasSpecular", material, g_config.uniforms.material.hasSpecular);
				SetConfigStrIfPresent("shininess", material, g_config.uniforms.material.shininess);
				SetConfigStrIfPresent("diffuseTexPrefix", material, g_config.uniforms.material.diffuseTexPrefix);
				SetConfigStrIfPresent("specularTexPrefix", material, g_config.uniforms.material.specularTexPrefix);
				if (material.contains("albedo"))
				{
					GData albedo = material.getGData("albedo");
					SetConfigStrIfPresent("ambient", albedo, g_config.uniforms.material.albedo.ambient);
					SetConfigStrIfPresent("diffuse", albedo, g_config.uniforms.material.albedo.diffuse);
					SetConfigStrIfPresent("specular", albedo, g_config.uniforms.material.albedo.specular);
				}
			}
		}
	}
}

std::string env::argv0()
{
	return g_exeLocation.generic_string();
}

std::string env::dirPath(Dir dir)
{
	switch (dir)
	{
	default:
	case env::Dir::Working:
		if (g_workingDir.empty())
		{
			g_workingDir = stdfs::current_path();
		}
		return g_workingDir.generic_string();
	case env::Dir::Executable:
		if (g_exePath.empty())
		{
			LOG_E("[Env] Unknown executable path! Using working directory instead [%s]", g_workingDir.generic_string().data());
			g_exePath = dirPath(Dir::Working);
		}
		return g_exePath.generic_string();
	}
}

std::vector<std::string_view> const& env::args()
{
	return g_args;
}

bool env::isDefined(std::string_view arg)
{
	return std::find_if(g_args.begin(), g_args.end(), [arg](std::string_view s) { return s == arg; }) != g_args.end();
}
} // namespace le
