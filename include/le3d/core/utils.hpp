#pragma once
#include <string>
#include <sstream>
#include <vector>
#include "le3d/stdtypes.hpp"

namespace le::utils
{
std::string readFile(std::string_view path);
std::vector<u8> readBytes(std::string_view path);
} // namespace le::utils
