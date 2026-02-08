#include "repaddu/format_analysis_json.h"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>

namespace repaddu::format
    {
    namespace
        {
        std::string escapeJson(const std::string& value)
            {
            std::string out;
            out.reserve(value.size() + 8);
            for (char ch : value)
                {
                switch (ch)
                    {
                    case '\"': out += "\\\""; break;
                    case '\\': out += "\\\\"; break;
                    case '\b': out += "\\b"; break;
                    case '\f': out += "\\f"; break;
                    case '\n': out += "\\n"; break;
                    case '\r': out += "\\r"; break;
                    case '\t': out += "\\t"; break;
                    default:
                        if (static_cast<unsigned char>(ch) < 0x20)
                            {
                            std::ostringstream hex;
                            hex << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                                << static_cast<int>(static_cast<unsigned char>(ch));
                            out += hex.str();
                            }
                        else
                            {
                            out += ch;
                            }
                        break;
                    }
                }
            return out;
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
        }

    std::string renderAnalysisJson(const core::CliOptions& options,
                                   const std::vector<core::FileEntry>& allFiles,
                                   const std::vector<std::size_t>& includedIndices)
        {
        std::uintmax_t totalSize = 0;
        for (const auto& f : allFiles)
            {
            totalSize += f.sizeBytes;
            }

        std::uintmax_t includedSize = 0;
        std::uintmax_t includedTokens = 0;
        std::vector<core::FileEntry> includedFiles;
        includedFiles.reserve(includedIndices.size());

        for (std::size_t idx : includedIndices)
            {
            const auto& f = allFiles[idx];
            includedSize += f.sizeBytes;
            std::uintmax_t tokens = f.tokenCount;
            if (tokens == 0 && f.sizeBytes > 0 && !f.isBinary)
                {
                tokens = f.sizeBytes / 4;
                }
            includedTokens += tokens;
            includedFiles.push_back(f);
            }

        std::map<std::string, std::size_t> languageCounts;
        std::map<std::string, std::size_t> buildFiles;
        std::size_t totalFiles = 0;

        for (const auto& entry : includedFiles)
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
        out << "{\n";
        out << "  \"type\": \"analysis_report\",\n";
        out << "  \"repository\": \"" << escapeJson(options.inputPath.string()) << "\",\n";
        out << "  \"overall\": {\n";
        out << "    \"total_files\": " << allFiles.size() << ",\n";
        out << "    \"total_size_bytes\": " << totalSize << "\n";
        out << "  },\n";
        out << "  \"included\": {\n";
        out << "    \"file_count\": " << includedIndices.size() << ",\n";
        out << "    \"size_bytes\": " << includedSize << ",\n";
        out << "    \"estimated_tokens\": " << includedTokens << "\n";
        out << "  },\n";
        out << "  \"languages\": [\n";
        std::size_t index = 0;
        for (const auto& entry : languageCounts)
            {
            const double percent = (totalFiles == 0)
                ? 0.0
                : (static_cast<double>(entry.second) * 100.0) / static_cast<double>(totalFiles);
            out << "    {\"name\": \"" << escapeJson(entry.first) << "\", \"count\": " << entry.second
                << ", \"percent\": " << std::fixed << std::setprecision(1) << percent << "}";
            if (index + 1 < languageCounts.size())
                {
                out << ",";
                }
            out << "\n";
            ++index;
            }
        out << "  ],\n";
        out << "  \"build_files\": [\n";
        index = 0;
        for (const auto& entry : buildFiles)
            {
            out << "    {\"name\": \"" << escapeJson(entry.first) << "\", \"count\": " << entry.second << "}";
            if (index + 1 < buildFiles.size())
                {
                out << ",";
                }
            out << "\n";
            ++index;
            }
        out << "  ]\n";
        out << "}\n";

        return out.str();
        }
    }
