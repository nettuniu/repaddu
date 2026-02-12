#include "repaddu/language_profiles.h"

#include "repaddu/core_types.h"

#include <algorithm>
#include <map>

namespace repaddu::core
    {
    namespace
        {
        std::string normalizeId(std::string_view value)
            {
            return toLowerCopy(value);
            }

        const std::vector<LanguageProfile>& languageProfiles()
            {
            static const std::vector<LanguageProfile> profiles =
                {
                    { "c", "C",
                        { ".c" },
                        { ".h" },
                        { "CMakeLists.txt", "Makefile", "meson.build", "BUILD", "BUILD.bazel" },
                        true },
                    { "cpp", "C++",
                        { ".cc", ".cpp", ".cxx" },
                        { ".h", ".hpp", ".hh", ".hxx" },
                        { "CMakeLists.txt", "Makefile", "meson.build", "BUILD", "BUILD.bazel" },
                        true },
                    { "rust", "Rust",
                        { ".rs" },
                        {},
                        { "Cargo.toml", "Cargo.lock", "rust-toolchain", "rust-toolchain.toml" },
                        false },
                    { "python", "Python",
                        { ".py", ".pyi" },
                        {},
                        { "pyproject.toml", "setup.py", "setup.cfg", "requirements.txt" },
                        false }
                };
            return profiles;
            }

        const std::vector<BuildSystemProfile>& buildSystemProfiles()
            {
            static const std::vector<BuildSystemProfile> profiles =
                {
                    { "cmake", { "CMakeLists.txt" } },
                    { "make", { "Makefile" } },
                    { "meson", { "meson.build" } },
                    { "bazel", { "BUILD", "BUILD.bazel" } },
                    { "cargo", { "Cargo.toml", "Cargo.lock", "rust-toolchain", "rust-toolchain.toml" } },
                    { "python", { "pyproject.toml", "setup.py", "setup.cfg", "requirements.txt" } }
                };
            return profiles;
            }

        void appendBuildFiles(const std::vector<std::string>& values,
            std::vector<std::string>& outList)
            {
            for (const std::string& value : values)
                {
                if (std::find(outList.begin(), outList.end(), value) == outList.end())
                    {
                    outList.push_back(value);
                    }
                }
            }
        }

    const LanguageProfile* findLanguageProfile(std::string_view id)
        {
        const std::string normalized = normalizeId(id);
        for (const auto& profile : languageProfiles())
            {
            if (profile.id == normalized)
                {
                return &profile;
                }
            }
        return nullptr;
        }

    const BuildSystemProfile* findBuildSystemProfile(std::string_view id)
        {
        const std::string normalized = normalizeId(id);
        for (const auto& profile : buildSystemProfiles())
            {
            if (profile.id == normalized)
                {
                return &profile;
                }
            }
        return nullptr;
        }

    DetectionResult detectLanguageAndBuildSystem(const std::vector<FileEntry>& files)
        {
        DetectionResult result;

        std::map<std::string, std::size_t> languageScores;
        bool hasCMake = false;
        bool hasMeson = false;
        bool hasBazel = false;
        bool hasCargo = false;
        bool hasPythonBuild = false;
        bool hasMake = false;

        for (const auto& file : files)
            {
            const std::string filename = toLowerCopy(file.relativePath.filename().string());
            if (filename == "cmakelists.txt")
                {
                hasCMake = true;
                }
            else if (filename == "meson.build")
                {
                hasMeson = true;
                }
            else if (filename == "build" || filename == "build.bazel")
                {
                hasBazel = true;
                }
            else if (filename == "cargo.toml" || filename == "cargo.lock"
                || filename == "rust-toolchain" || filename == "rust-toolchain.toml")
                {
                hasCargo = true;
                }
            else if (filename == "pyproject.toml" || filename == "setup.py"
                || filename == "setup.cfg" || filename == "requirements.txt")
                {
                hasPythonBuild = true;
                }
            else if (filename == "makefile")
                {
                hasMake = true;
                }

            if (file.isBinary)
                {
                continue;
                }

            for (const auto& profile : languageProfiles())
                {
                bool matched = std::find(profile.sourceExtensions.begin(), profile.sourceExtensions.end(),
                    file.extensionLower) != profile.sourceExtensions.end();
                if (!matched && profile.supportsHeaders)
                    {
                    matched = std::find(profile.headerExtensions.begin(), profile.headerExtensions.end(),
                        file.extensionLower) != profile.headerExtensions.end();
                    }
                if (matched)
                    {
                    languageScores[profile.id] += 1;
                    }
                }
            }

        const std::vector<std::string> languagePriority = { "cpp", "c", "rust", "python" };
        std::size_t bestScore = 0;
        for (const auto& id : languagePriority)
            {
            const std::size_t score = languageScores[id];
            if (score > bestScore)
                {
                bestScore = score;
                result.languageId = id;
                }
            }

        if (hasCargo)
            {
            result.buildSystemId = "cargo";
            }
        else if (hasCMake)
            {
            result.buildSystemId = "cmake";
            }
        else if (hasMeson)
            {
            result.buildSystemId = "meson";
            }
        else if (hasBazel)
            {
            result.buildSystemId = "bazel";
            }
        else if (hasPythonBuild)
            {
            result.buildSystemId = "python";
            }
        else if (hasMake)
            {
            result.buildSystemId = "make";
            }

        return result;
        }

    std::vector<std::string> resolveBuildFileNames(const CliOptions& options)
        {
        std::vector<std::string> result;

        if (!options.buildSystem.empty())
            {
            const BuildSystemProfile* buildProfile = findBuildSystemProfile(options.buildSystem);
            if (buildProfile != nullptr)
                {
                appendBuildFiles(buildProfile->buildFiles, result);
                }
            }

        if (!options.language.empty())
            {
            const LanguageProfile* languageProfile = findLanguageProfile(options.language);
            if (languageProfile != nullptr)
                {
                appendBuildFiles(languageProfile->buildFiles, result);
                }
            }

        if (result.empty())
            {
            // Default: Include all known build system files
            for (const auto& profile : buildSystemProfiles())
                {
                appendBuildFiles(profile.buildFiles, result);
                }
            }

        return result;
        }
    }
