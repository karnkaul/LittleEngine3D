#pragma once
#include <string>
#include "le3d/core/version.hpp"
#include "stdtypes.hpp"

namespace le::versions
{
const Version engineVersion();
const std::string_view gitCommitHash();
const std::string_view buildVersion();
} // namespace le::versions
