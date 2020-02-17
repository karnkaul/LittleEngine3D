#include "le3d/engine/engine_loop.hpp"

using namespace le;

s32 main(s32 argc, char const** argv)
{
	s32 ret = le::engineLoop::run(argc, argv);
	return ret;
}
