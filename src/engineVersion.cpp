#include "le3d/engineVersion.hpp"
#include "buildVersion.hpp"

namespace le::versions
{
const Version engineVersion(LE3D_VERSION);
const std::string_view gitCommitHash = LE3D_GIT_COMMIT_HASH;
const std::string_view buildVersion = LE3D_BUILD_VERSION;
} // namespace le::versions
