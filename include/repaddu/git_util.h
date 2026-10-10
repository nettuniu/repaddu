#ifndef REPADDU_GIT_UTIL_H
#define REPADDU_GIT_UTIL_H

#include <filesystem>
#include <string>

namespace repaddu::git
    {
    // @note Returns commit hash or empty string if not in a git repository
    std::string detectGitCommit(const std::filesystem::path& repoRoot);

    // @note Returns branch name or empty string if not available
    std::string detectGitBranch(const std::filesystem::path& repoRoot);
    }

#endif // REPADDU_GIT_UTIL_H
