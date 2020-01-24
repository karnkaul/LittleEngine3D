#pragma once
#include <string>
#include "le3d/core/version.hpp"

namespace le::env
{
Version const engineVersion();
std::string_view const gitCommitHash();
std::string_view const buildVersion();
} // namespace le::env
