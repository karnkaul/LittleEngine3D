#include "le3d/env/threads.hpp"
#include "app/gameloop.hpp"

using namespace le;

s32 main(s32 argc, char const** argv)
{
	s32 ret = letest::gameloop::run(argc, argv);
	threads::joinAll();
	return ret;
}
