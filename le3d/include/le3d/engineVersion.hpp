#pragma once
#include <string>
#include "le3d/core/version.hpp"
#include "stdtypes.hpp"

namespace le::versions
{
Version const engineVersion();
std::string_view const gitCommitHash();
std::string_view const buildVersion();
} // namespace le::versions
