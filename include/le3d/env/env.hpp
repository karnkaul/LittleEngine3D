#pragma once
#include <string>
#include <vector>
#include "le3d/stdtypes.hpp"

namespace le
{
namespace env
{
void init(s32 argc, char** argv);
std::string_view pwd();
std::string fullPath(std::string_view relative);
const std::vector<std::string_view>& args();
bool isDefined(std::string_view arg);
} // namespace env
} // namespace le
