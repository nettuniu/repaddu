#include "repaddu/format_language_report.h"

#include "repaddu/core_types.h"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>

namespace repaddu::format
    {
    namespace
        {
        std::string languageForExtension(const std::string& extensionLower)
            {
            if (extensionLower == ".c" || extensionLower == ".h")
                {
                return "C";
                }
            if (extensionLower == ".cc" || extensionLower == ".cpp" || extensionLower == ".cxx"
                || extensionLower == ".hpp" || extensionLower == ".hh" || extensionLower == ".hxx")
                {
                return "C++";
                }
            if (extensionLower == ".rs")
                {
                return "Rust";
                }
            if (extensionLower == ".py" || extensionLower == ".pyi")
                {
                return "Python";
                }
            return "Other";
            }

        std::string normalizeName(std::string value)
            {
            std::transform(value.begin(), value.end(), value.begin(),
                [](unsigned char ch)
                {
                return static_cast<char>(std::tolower(ch));
                });
            return value;
            }
        }

    std::string renderLanguageReport(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files)
        {
        std::map<std::string, std::size_t> languageCounts;
        std::map<std::string, std::size_t> buildFiles;
        std::size_t totalFiles = 0;

        for (const auto& entry : files)
            {
            if (!options.includeBinaries && entry.isBinary)
                {
                continue;
                }

            const std::string language = languageForExtension(entry.extensionLower);
            languageCounts[language] += 1;
            ++totalFiles;

            const std::string filename = normalizeName(entry.relativePath.filename().string());
            if (filename == "cmakelists.txt")
                {
                buildFiles["CMakeLists.txt"] += 1;
                }
            else if (filename == "meson.build")
                {
                buildFiles["meson.build"] += 1;
                }
            else if (filename == "makefile")
                {
                buildFiles["Makefile"] += 1;
                }
            else if (filename == "build" || filename == "build.bazel")
                {
                buildFiles["Bazel BUILD"] += 1;
                }
            else if (filename == "cargo.toml")
                {
                buildFiles["Cargo.toml"] += 1;
                }
            else if (filename == "cargo.lock")
                {
                buildFiles["Cargo.lock"] += 1;
                }
            else if (filename == "rust-toolchain" || filename == "rust-toolchain.toml")
                {
                buildFiles["rust-toolchain"] += 1;
                }
            else if (filename == "pyproject.toml")
                {
                buildFiles["pyproject.toml"] += 1;
                }
            else if (filename == "setup.py")
                {
                buildFiles["setup.py"] += 1;
                }
            else if (filename == "setup.cfg")
                {
                buildFiles["setup.cfg"] += 1;
                }
            else if (filename == "requirements.txt")
                {
                buildFiles["requirements.txt"] += 1;
                }
            }

        std::ostringstream out;
        out << "Language scan report\n";
        out << "====================\n\n";
        out << "Total files counted: " << totalFiles << "\n\n";

        if (totalFiles == 0)
            {
            out << "No files matched the scan criteria.\n";
            return out.str();
            }

        out << "By language (file count)\n";
        out << "------------------------\n";
        out << std::fixed << std::setprecision(1);
        for (const auto& entry : languageCounts)
            {
            const double percent = (static_cast<double>(entry.second) * 100.0)
                / static_cast<double>(totalFiles);
            out << entry.first << ": " << entry.second << " (" << percent << "%)\n";
            }

        out << "\nBuild-system files detected\n";
        out << "----------------------------\n";
        if (buildFiles.empty())
            {
            out << "None\n";
            }
        else
            {
            for (const auto& entry : buildFiles)
                {
                out << entry.first << ": " << entry.second << "\n";
                }
            }

        out << "\nNotes:\n";
        out << "- Hidden files are excluded unless --include-hidden is used.\n";
        out << "- Binary files are excluded unless --include-binaries is used.\n";

        return out.str();
        }
    }
