#pragma once 
#include < string>
#include <vector>
#include "le3d/stdtypes.hpp"

namespace le
{
namespace env
{
void init(s32 argc, char** argv);
const std::vector<std::string_view>& args();
bool isDefined(std::string_view arg);
}
}
