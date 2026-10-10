#include "repaddu/git_util.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace repaddu::git
    {
    namespace
        {
        std::string trim(std::string_view text)
            {
            std::size_t start = 0;
            while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start])) != 0)
                {
                ++start;
                }
            std::size_t end = text.size();
            while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0)
                {
                --end;
                }
            return std::string(text.substr(start, end - start));
            }

        std::filesystem::path resolveGitDir(const std::filesystem::path& repoRoot)
            {
            std::error_code ec;
            std::filesystem::path gitPath = repoRoot / ".git";
            if (!std::filesystem::exists(gitPath, ec))
                {
                return {};
                }

            if (std::filesystem::is_regular_file(gitPath, ec))
                {
                std::ifstream stream(gitPath);
                std::string line;
                if (std::getline(stream, line))
                    {
                    const std::string trimmed = trim(line);
                    constexpr std::string_view prefix = "gitdir:";
                    if (trimmed.starts_with(prefix))
                        {
                        const std::string dirPart = trim(trimmed.substr(prefix.size()));
                        std::filesystem::path target(dirPart);
                        if (target.is_relative())
                            {
                            target = repoRoot / target;
                            }
                        if (std::filesystem::is_directory(target, ec))
                            {
                            return target;
                            }
                        }
                    }
                return {};
                }

            if (std::filesystem::is_directory(gitPath, ec))
                {
                return gitPath;
                }

            return {};
            }

        std::string findInPackedRefs(const std::filesystem::path& gitDir, std::string_view refName)
            {
            const std::filesystem::path packedRefs = gitDir / "packed-refs";
            std::ifstream stream(packedRefs);
            if (!stream.is_open())
                {
                return {};
                }

            std::string line;
            while (std::getline(stream, line))
                {
                if (line.empty() || line[0] == '#' || line[0] == '^')
                    {
                    continue;
                    }
                const auto spacePos = line.find(' ');
                if (spacePos != std::string::npos)
                    {
                    const std::string commit = line.substr(0, spacePos);
                    const std::string ref = trim(line.substr(spacePos + 1));
                    if (ref == refName)
                        {
                        return commit;
                        }
                    }
                }
            return {};
            }
        }

    std::string detectGitCommit(const std::filesystem::path& repoRoot)
        {
        const std::filesystem::path gitDir = resolveGitDir(repoRoot);
        if (gitDir.empty())
            {
            return {};
            }

        const std::filesystem::path headFile = gitDir / "HEAD";
        std::ifstream stream(headFile);
        if (!stream.is_open())
            {
            return {};
            }

        std::string headLine;
        if (!std::getline(stream, headLine))
            {
            return {};
            }

        const std::string trimmed = trim(headLine);
        constexpr std::string_view refPrefix = "ref: ";
        if (trimmed.starts_with(refPrefix))
            {
            const std::string refPath = trim(trimmed.substr(refPrefix.size()));
            const std::filesystem::path fullRefFile = gitDir / refPath;
            std::ifstream refStream(fullRefFile);
            if (refStream.is_open())
                {
                std::string commitLine;
                if (std::getline(refStream, commitLine))
                    {
                    return trim(commitLine);
                    }
                }
            return findInPackedRefs(gitDir, refPath);
            }

        if (trimmed.size() >= 40)
            {
            return trimmed;
            }

        return {};
        }

    std::string detectGitBranch(const std::filesystem::path& repoRoot)
        {
        const std::filesystem::path gitDir = resolveGitDir(repoRoot);
        if (gitDir.empty())
            {
            return {};
            }

        const std::filesystem::path headFile = gitDir / "HEAD";
        std::ifstream stream(headFile);
        if (!stream.is_open())
            {
            return {};
            }

        std::string headLine;
        if (!std::getline(stream, headLine))
            {
            return {};
            }

        const std::string trimmed = trim(headLine);
        constexpr std::string_view refPrefix = "ref: ";
        if (trimmed.starts_with(refPrefix))
            {
            const std::string refPath = trim(trimmed.substr(refPrefix.size()));
            constexpr std::string_view headsPrefix = "refs/heads/";
            if (refPath.starts_with(headsPrefix))
                {
                return refPath.substr(headsPrefix.size());
                }
            return refPath;
            }

        return "HEAD (detached)";
        }
    }
