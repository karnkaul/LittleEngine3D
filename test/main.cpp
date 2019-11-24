#include "le3d/env/threads.hpp"
#include "app/gameloop.hpp"

s32 main(s32 argc, char** argv)
{
	s32 ret = letest::gameloop::run(argc, argv);
	le::threads::joinAll();
	return ret;
}
