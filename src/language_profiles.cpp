#include "repaddu/language_profiles.h"

#include "repaddu/core_types.h"

#include <algorithm>

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
