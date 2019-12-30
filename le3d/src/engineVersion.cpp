#include "le3d/engineVersion.hpp"
#include "buildVersion.hpp"

namespace le::versions
{
const Version engineVersion()
{
	static Version s_engineVersion(LE3D_VERSION);
	return s_engineVersion;
}

const std::string_view gitCommitHash()
{
	static std::string_view s_gitCommitHash = LE3D_GIT_COMMIT_HASH;
	return s_gitCommitHash;
}

const std::string_view buildVersion()
{
	static std::string_view s_buildVersion = LE3D_BUILD_VERSION;
	return s_buildVersion;
}
} // namespace le::versions
