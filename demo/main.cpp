#include "app/gameloop.hpp"

using namespace le;

s32 main(s32 argc, char const** argv)
{
	s32 ret = letest::gameloop::run(argc, argv);
	return ret;
}
