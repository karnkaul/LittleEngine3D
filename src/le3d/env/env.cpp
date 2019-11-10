#include <algorithm>
#include "le3d/env/env.hpp"

namespace le
{
namespace
{
std::string_view g_exePath;
std::vector<std::string_view> g_args;
}

void env::init(s32 argc, char** argv)
{
	if (argc > 0)
	{
		g_exePath = argv[0];
		for (s32 i = 1; i < argc; ++i)
		{
			g_args.push_back(argv[i]);
		}
	}
}

const std::vector<std::string_view>& env::args()
{
	return g_args;
}

bool env::isDefined(std::string_view arg)
{
	return std::find_if(g_args.begin(), g_args.end(), [arg](std::string_view s) { return s == arg; }) != g_args.end();
}
}
