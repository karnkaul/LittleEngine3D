#pragma once
#include <string>
#include "le3d/core/version.hpp"
#include "stdtypes.hpp"

namespace le::versions
{
extern const Version engineVersion;
extern const std::string_view gitCommitHash;
extern const std::string_view buildVersion;
}
