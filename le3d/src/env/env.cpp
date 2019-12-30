#include <algorithm>
#if defined(__linux__)
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#endif
#include "le3d/core/log.hpp"
#include "le3d/core/gdata.hpp"
#include "le3d/env/env.hpp"
#include "env/threadsImpl.hpp"

namespace le
{
namespace env
{
EngineConfig g_config;
std::string g_EOL = "\n";
} // namespace env

namespace
{
stdfs::path g_exeLocation;
stdfs::path g_exePath;
stdfs::path g_workingDir;
std::vector<std::string_view> g_args;

void SetConfigStrIfPresent(const std::string& id, const GData& data, std::string& member)
{
	if (data.contains(id))
	{
		member = data.getString(id);
		LOG_D("[EngineConfig] Extracted [%s] = [%s]", id.data(), member.data());
	}
}

// void SetConfigS32IfPresent(const std::string& id, const GData& data, s32& member)
//{
//	if (data.contains(id))
//	{
//		member = data.getS32(id);
//		LOG_D("[EngineConfig] Extracted [%s] = [%d]", id.data(), member);
//	}
//}
} // namespace

void env::init(s32 argc, char** argv)
{
#if defined(__linux__)
	s32 threadStatus = XInitThreads();
	if (threadStatus == 0)
	{
		LOG_E("[OS] ERROR calling XInitThreads()! UB follows.");
		threadsImpl::g_maxThreads = 1;
	}
#endif
	if (argc > 0)
	{
		g_exeLocation = argv[0];
		g_exePath = g_exeLocation.parent_path();
		for (s32 i = 1; i < argc; ++i)
		{
			g_args.push_back(argv[i]);
		}
	}
	g_workingDir = std::filesystem::current_path();
}

void env::setConfig(std::string json)
{
	GData data(json);
	if (data.fieldCount() > 0)
	{
		if (data.contains("uniforms"))
		{
			GData uniforms = data.getGData("uniforms");
			SetConfigStrIfPresent("material", uniforms, g_config.uniforms.material);
			SetConfigStrIfPresent("shininess", uniforms, g_config.uniforms.shininess);
			SetConfigStrIfPresent("tint", uniforms, g_config.uniforms.tint);
			SetConfigStrIfPresent("diffuseTexPrefix", uniforms, g_config.uniforms.diffuseTexPrefix);
			SetConfigStrIfPresent("specularTexPrefix", uniforms, g_config.uniforms.specularTexPrefix);
			SetConfigStrIfPresent("modelMatrix", uniforms, g_config.uniforms.modelMatrix);
			SetConfigStrIfPresent("normalMatrix", uniforms, g_config.uniforms.normalMatrix);
		}
	}
}

stdfs::path env::dirPath(Dir dir)
{
	return dir == Dir::Executable ? g_exePath : g_workingDir;
}

const std::vector<std::string_view>& env::args()
{
	return g_args;
}

bool env::isDefined(std::string_view arg)
{
	return std::find_if(g_args.begin(), g_args.end(), [arg](std::string_view s) { return s == arg; }) != g_args.end();
}
} // namespace le
