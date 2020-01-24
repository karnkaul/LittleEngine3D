#include "le3d/env/engineVersion.hpp"
#include "buildVersion.hpp"

namespace le::env
{
Version const engineVersion()
{
	static Version s_engineVersion(LE3D_VERSION);
	return s_engineVersion;
}

std::string_view const gitCommitHash()
{
	static std::string_view s_gitCommitHash = LE3D_GIT_COMMIT_HASH;
	return s_gitCommitHash;
}

std::string_view const buildVersion()
{
	static std::string_view s_buildVersion = LE3D_BUILD_VERSION;
	return s_buildVersion;
}
} // namespace le::env
