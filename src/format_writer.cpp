#include "repaddu/format_writer.h"

#include "repaddu/core_types.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace repaddu::format
    {
    namespace
        {
        std::string padNumber(int value, int width)
            {
            std::ostringstream out;
            out.fill('0');
            out.width(width);
            out << value;
            return out.str();
            }

        std::string readFileContent(const std::filesystem::path& path, core::RunResult& outResult)
            {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
                {
                outResult = { core::ExitCode::io_failure, "Failed to open file for reading." };
                return {};
                }
            std::ostringstream buffer;
            buffer << stream.rdbuf();
            outResult = { core::ExitCode::success, "" };
            return buffer.str();
            }

        std::string escapeQuotes(std::string value)
            {
            std::string result;
            result.reserve(value.size());
            for (char ch : value)
                {
                if (ch == '"')
                    {
                    result.push_back('\\');
                    }
                result.push_back(ch);
                }
            return result;
            }

        std::string markerBlock(const core::FileEntry& entry, const std::string& content,
            core::MarkerMode mode)
            {
            std::ostringstream out;
            const std::string relative = entry.relativePath.generic_string();
            if (mode == core::MarkerMode::fenced)
                {
                out << "```repaddu-file\n";
                out << "path: " << relative << "\n";
                out << "bytes: " << entry.sizeBytes << "\n";
                out << "class: " << core::fileClassLabel(entry.fileClass) << "\n";
                out << "```\n";
                out << content;
                if (!content.empty() && content.back() != '\n')
                    {
                    out << "\n";
                    }
                out << "```\n";
                }
            else
                {
                out << "@@@ REPADDU FILE BEGIN path=\"" << escapeQuotes(relative)
                    << "\" bytes=" << entry.sizeBytes
                    << " class=" << core::fileClassLabel(entry.fileClass) << " @@@\n";
                out << content;
                if (!content.empty() && content.back() != '\n')
                    {
                    out << "\n";
                    }
                out << "@@@ REPADDU FILE END @@@\n";
                }
            return out.str();
            }

        std::string overviewTemplate(const std::string& overviewName, core::MarkerMode mode)
            {
            std::ostringstream out;
            out << "# repaddu output specification\n\n";
            out << "This file explains the layout and markers used in every other output file.\n";
            out << "All other files start with a short note: \"Read \"" << overviewName
                << "\" first for format and conventions.\"\n\n";

            out << "## Boundary markers\n";
            if (mode == core::MarkerMode::fenced)
                {
                out << "Files are wrapped in fenced blocks using the following pattern:\n\n";
                out << "```text\n";
                out << "```repaddu-file\n";
                out << "path: relative/path.ext\n";
                out << "bytes: 123\n";
                out << "class: header|source|other\n";
                out << "```\n";
                out << "<file content>\n";
                out << "```\n";
                out << "```\n\n";
                }
            else
                {
                out << "Files are wrapped in sentinel lines using the following pattern:\n\n";
                out << "```text\n";
                out << "@@@ REPADDU FILE BEGIN path=\"relative/path.ext\" bytes=123 class=header @@@\n";
                out << "<file content>\n";
                out << "@@@ REPADDU FILE END @@@\n";
                out << "```\n\n";
                }

            out << "## Output ordering\n";
            out << "1) This overview file.\n";
            out << "2) Optional tree listing file if enabled.\n";
            out << "3) Optional aggregated CMakeLists file if enabled.\n";
            out << "4) One or more grouped content files based on the chosen grouping strategy.\n\n";

            out << "## Example encoded file\n";
            out << "```text\n";
            if (mode == core::MarkerMode::fenced)
                {
                out << "```repaddu-file\n";
                out << "path: src/main.cpp\n";
                out << "bytes: 42\n";
                out << "class: source\n";
                out << "```\n";
                out << "int main() { return 0; }\n";
                out << "```\n";
                }
            else
                {
                out << "@@@ REPADDU FILE BEGIN path=\"src/main.cpp\" bytes=42 class=source @@@\n";
                out << "int main() { return 0; }\n";
                out << "@@@ REPADDU FILE END @@@\n";
                }
            out << "```\n";

            return out.str();
            }

        std::string boilerplateLine(const std::string& overviewName)
            {
            std::ostringstream out;
            out << "Read \"" << overviewName << "\" first for format and conventions.\n\n";
            return out.str();
            }

        core::OutputContent buildTreeOutput(const core::CliOptions& options,
            const std::string& overviewName,
            const std::string& treeListing,
            int index)
            {
            core::OutputContent output;
            output.filename = padNumber(index, options.numberWidth) + "_tree.md";
            std::ostringstream out;
            out << boilerplateLine(overviewName);
            out << "# Repository tree listing\n\n";
            out << "```text\n";
            out << treeListing;
            out << "```\n";
            output.content = out.str();
            output.contentBytes = static_cast<std::uintmax_t>(output.content.size());
            return output;
            }

        core::OutputContent buildCMakeOutput(const core::CliOptions& options,
            const std::string& overviewName,
            const std::vector<std::filesystem::path>& cmakeLists,
            const std::vector<core::FileEntry>& files,
            int index,
            core::RunResult& outResult)
            {
            core::OutputContent output;
            output.filename = padNumber(index, options.numberWidth) + "_cmake.md";

            std::ostringstream out;
            out << boilerplateLine(overviewName);
            out << "# Aggregated CMakeLists.txt files\n\n";
            if (cmakeLists.empty())
                {
                out << "No CMakeLists.txt files were found.\n";
                }
            else
                {
                for (const auto& path : cmakeLists)
                    {
                    core::RunResult readResult;
                    const std::filesystem::path absolute = options.inputPath / path;
                    const std::string content = readFileContent(absolute, readResult);
                    if (readResult.code != core::ExitCode::success)
                        {
                        outResult = readResult;
                        return {};
                        }

                    core::FileEntry entry;
                    entry.relativePath = path;
                    entry.sizeBytes = static_cast<std::uintmax_t>(content.size());
                    entry.fileClass = core::classifyExtension(core::toLowerCopy(entry.relativePath.extension().string()));
                    out << markerBlock(entry, content, options.markers) << "\n";
                    }
                }

            output.content = out.str();
            output.contentBytes = static_cast<std::uintmax_t>(output.content.size());
            outResult = { core::ExitCode::success, "" };
            return output;
            }

        void appendChunkOutputs(const core::CliOptions& options,
            const std::string& overviewName,
            const core::OutputChunk& chunk,
            const std::vector<core::FileEntry>& files,
            std::vector<core::OutputContent>& outputs,
            int& index,
            core::RunResult& outResult)
            {
            std::ostringstream header;
            header << boilerplateLine(overviewName);
            header << "# " << chunk.title << "\n\n";

            std::ostringstream current;
            current << header.str();

            std::uintmax_t currentBytes = static_cast<std::uintmax_t>(current.str().size());
            int part = 1;

            for (std::size_t fileIndex : chunk.fileIndices)
                {
                core::RunResult readResult;
                const core::FileEntry& entry = files[fileIndex];
                const std::string content = readFileContent(entry.absolutePath, readResult);
                if (readResult.code != core::ExitCode::success)
                    {
                    outResult = readResult;
                    return;
                    }
                const std::string block = markerBlock(entry, content, options.markers) + "\n";
                const std::uintmax_t blockBytes = static_cast<std::uintmax_t>(block.size());

                if (options.maxBytes > 0 && !chunk.fileIndices.empty()
                    && currentBytes + blockBytes > options.maxBytes)
                    {
                    core::OutputContent output;
                    const std::string partSuffix = (part > 1) ? "_part" + std::to_string(part) : "";
                    output.filename = padNumber(index, options.numberWidth) + "_" + chunk.category + partSuffix + ".md";
                    output.content = current.str();
                    output.contentBytes = currentBytes;
                    outputs.push_back(std::move(output));

                    ++index;
                    ++part;
                    current.str(std::string());
                    current.clear();
                    current << header.str();
                    currentBytes = static_cast<std::uintmax_t>(current.str().size());
                    }

                current << block;
                currentBytes += blockBytes;

                if (options.maxBytes > 0 && blockBytes > options.maxBytes)
                    {
                    outResult = { core::ExitCode::output_constraints, "A single file block exceeds --max-bytes." };
                    return;
                    }
                }

            if (currentBytes > 0)
                {
                core::OutputContent output;
                const std::string partSuffix = (part > 1) ? "_part" + std::to_string(part) : "";
                output.filename = padNumber(index, options.numberWidth) + "_" + chunk.category + partSuffix + ".md";
                output.content = current.str();
                output.contentBytes = currentBytes;
                outputs.push_back(std::move(output));
                ++index;
                }

            outResult = { core::ExitCode::success, "" };
            }
        }

    core::RunResult writeOutputs(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const std::vector<core::OutputChunk>& chunks,
        const std::string& treeListing,
        const std::vector<std::filesystem::path>& cmakeLists)
        {
        std::error_code errorCode;
        std::filesystem::create_directories(options.outputPath, errorCode);
        if (errorCode)
            {
            return { core::ExitCode::io_failure, "Failed to create output directory." };
            }

        std::vector<core::OutputContent> outputs;
        const std::string overviewName = padNumber(0, options.numberWidth) + "_overview.md";

        core::OutputContent overview;
        overview.filename = overviewName;
        overview.content = overviewTemplate(overviewName, options.markers);
        overview.contentBytes = static_cast<std::uintmax_t>(overview.content.size());
        outputs.push_back(std::move(overview));

        int index = 1;
        if (options.emitTree)
            {
            outputs.push_back(buildTreeOutput(options, overviewName, treeListing, index));
            ++index;
            }

        if (options.emitCMake)
            {
            core::RunResult cmakeResult;
            outputs.push_back(buildCMakeOutput(options, overviewName, cmakeLists, files, index, cmakeResult));
            if (cmakeResult.code != core::ExitCode::success)
                {
                return cmakeResult;
                }
            ++index;
            }

        for (const auto& chunk : chunks)
            {
            core::RunResult chunkResult;
            appendChunkOutputs(options, overviewName, chunk, files, outputs, index, chunkResult);
            if (chunkResult.code != core::ExitCode::success)
                {
                return chunkResult;
                }
            }

        if (options.maxFiles > 0 && static_cast<int>(outputs.size()) > options.maxFiles)
            {
            return { core::ExitCode::output_constraints, "Output file count exceeds --max-files." };
            }

        for (const auto& output : outputs)
            {
            if (options.maxBytes > 0 && output.contentBytes > options.maxBytes)
                {
                return { core::ExitCode::output_constraints, "Output file exceeds --max-bytes." };
                }

            const std::filesystem::path outPath = options.outputPath / output.filename;
            std::ofstream stream(outPath, std::ios::binary);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            stream << output.content;
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to flush output file." };
                }
            }

        return { core::ExitCode::success, "" };
        }
    }
