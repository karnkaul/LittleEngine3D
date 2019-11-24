#include <algorithm>
#if defined(__linux__)
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#endif
#include "le3d/core/log.hpp"
#include "le3d/env/env.hpp"
#include "le3d/env/threads.hpp"

namespace le
{
namespace
{
std::string_view g_exePath;
std::string_view g_pwd;
std::vector<std::string_view> g_args;
} // namespace

namespace threads
{
extern u32 g_maxThreads;
}

void env::init(s32 argc, char** argv)
{
#if defined(__linux__)
	s32 threadStatus = XInitThreads();
	if (threadStatus == 0)
	{
		LOG_E("[OS] ERROR calling XInitThreads()! UB follows.");
		threads::g_maxThreads = 1;
	}
#endif
	if (argc > 0)
	{
		g_exePath = argv[0];
		std::string_view token;
#if defined(_WIN64)
		token = "\\";
#else
		token = "/";
#endif
		g_pwd = g_exePath.substr(0, g_exePath.find_last_of(token));
		for (s32 i = 1; i < argc; ++i)
		{
			g_args.push_back(argv[i]);
		}
	}
}

std::string_view env::pwd()
{
	return g_exePath;
}

std::string env::fullPath(std::string_view relative)
{
	std::string ret(g_pwd);
	ret += "/";
	ret += relative;
	return ret;
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
