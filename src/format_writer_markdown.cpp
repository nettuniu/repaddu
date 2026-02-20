#include "format_writer_internal.h"

#include "format_writer_alt_formats.h"

#include "repaddu/analysis_tokens.h"

#include <sstream>

namespace repaddu::format::detail
    {
    namespace
        {
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
        }

    bool ContentCache::tryGet(const std::filesystem::path& path, std::string& outContent) const
        {
        const std::string key = path.string();
        auto it = entries.find(key);
        if (it == entries.end())
            {
            return false;
            }
        outContent = it->second;
        return true;
        }

    void ContentCache::store(const std::filesystem::path& path, const std::string& content)
        {
        if (content.size() > maxBytes)
            {
            return;
            }
        if (totalBytes + static_cast<std::uintmax_t>(content.size()) > maxBytes)
            {
            return;
            }
        const std::string key = path.string();
        if (entries.find(key) != entries.end())
            {
            return;
            }
        entries.emplace(key, content);
        totalBytes += static_cast<std::uintmax_t>(content.size());
        }

    std::string padNumber(int value, int width)
        {
        std::ostringstream out;
        out.fill('0');
        out.width(width);
        out << value;
        return out.str();
        }

    void writeChunkMarkerBlock(OutputWriter& writer,
        const core::FileEntry& entry,
        const std::string& content,
        core::MarkerMode mode,
        bool emitFrontmatter)
        {
        const std::string relative = entry.relativePath.generic_string();
        if (mode == core::MarkerMode::fenced)
            {
            writer.write("```repaddu-file\n");
            writer.write("path: ");
            writer.write(relative);
            writer.write("\n");
            writer.write("bytes: ");
            writer.write(std::to_string(entry.sizeBytes));
            writer.write("\n");
            writer.write("tokens: ");
            writer.write(std::to_string(entry.tokenCount));
            writer.write("\n");
            writer.write("class: ");
            writer.write(core::fileClassLabel(entry.fileClass));
            writer.write("\n");
            writer.write("```\n");
            if (emitFrontmatter)
                {
                writer.write("---\n");
                writer.write("path: ");
                writer.write(relative);
                writer.write("\n");
                writer.write("bytes: ");
                writer.write(std::to_string(entry.sizeBytes));
                writer.write("\n");
                writer.write("tokens: ");
                writer.write(std::to_string(entry.tokenCount));
                writer.write("\n");
                writer.write("class: ");
                writer.write(core::fileClassLabel(entry.fileClass));
                writer.write("\n");
                writer.write("---\n");
                }
            writer.write(content);
            if (!content.empty() && content.back() != '\n')
                {
                writer.write("\n");
                }
            writer.write("```\n");
            }
        else
            {
            writer.write("@@@ REPADDU FILE BEGIN path=\"");
            writer.write(escapeQuotes(relative));
            writer.write("\" bytes=");
            writer.write(std::to_string(entry.sizeBytes));
            writer.write(" tokens=");
            writer.write(std::to_string(entry.tokenCount));
            writer.write(" class=");
            writer.write(core::fileClassLabel(entry.fileClass));
            writer.write(" @@@\n");
            if (emitFrontmatter)
                {
                writer.write("---\n");
                writer.write("path: ");
                writer.write(relative);
                writer.write("\n");
                writer.write("bytes: ");
                writer.write(std::to_string(entry.sizeBytes));
                writer.write("\n");
                writer.write("tokens: ");
                writer.write(std::to_string(entry.tokenCount));
                writer.write("\n");
                writer.write("class: ");
                writer.write(core::fileClassLabel(entry.fileClass));
                writer.write("\n");
                writer.write("---\n");
                }
            writer.write(content);
            if (!content.empty() && content.back() != '\n')
                {
                writer.write("\n");
                }
            writer.write("@@@ REPADDU FILE END @@@\n");
            }
        }

    std::string overviewTemplate(const std::string& overviewName,
        core::MarkerMode mode,
        bool emitBuildFiles,
        bool emitLinks,
        const std::vector<std::string>& generatedFiles)
        {
        std::ostringstream out;
        out << "# repaddu output specification\n\n";
        out << "This file explains the layout and markers used in every other output file.\n";
        out << "All other files start with a short note: \"Read \"" << overviewName
            << "\" first for format and conventions.\"\n\n";

        out << "## Table of Contents\n";
        for (const auto& filename : generatedFiles)
            {
            if (filename == overviewName) continue;
            if (emitLinks)
                {
                out << "- [" << filename << "](" << filename << ")\n";
                }
            else
                {
                out << "- " << filename << "\n";
                }
            }
        out << "\n";

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
        if (emitBuildFiles)
            {
            out << "4) Optional aggregated build-context files if enabled.\n";
            out << "5) One or more grouped content files based on the chosen grouping strategy.\n\n";
            }
        else
            {
            out << "4) One or more grouped content files based on the chosen grouping strategy.\n\n";
            }

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

    core::RunResult readContentWithCache(const std::filesystem::path& path,
        const std::string& relativePath,
        security::PiiRedactor* redactor,
        ContentCache* cache,
        std::string& outContent,
        std::uintmax_t* outTokens)
        {
        if (cache && cache->tryGet(path, outContent))
            {
            if (outTokens)
                {
                *outTokens = analysis::TokenEstimator::estimateTokens(outContent);
                }
            return { core::ExitCode::success, "" };
            }

        core::RunResult readResult;
        outContent = readFileContent(path, readResult, outTokens, redactor, relativePath);
        if (readResult.code != core::ExitCode::success)
            {
            return readResult;
            }
        if (cache)
            {
            cache->store(path, outContent);
            }
        return { core::ExitCode::success, "" };
        }

    void writeTreeOutput(OutputWriter& writer,
        const std::string& overviewName,
        const std::string& treeListing)
        {
        writer.write(boilerplateLine(overviewName));
        writer.write("# Repository tree listing\n\n");
        writer.write("```text\n");
        writer.write(treeListing);
        writer.write("```\n");
        }

    core::RunResult writeAggregatedFilesOutput(OutputWriter& writer,
        const std::string& overviewName,
        const std::string& title,
        const std::string& emptyMessage,
        const std::vector<std::filesystem::path>& paths,
        const core::CliOptions& options,
        security::PiiRedactor* redactor,
        ContentCache* cache)
        {
        writer.write(boilerplateLine(overviewName));
        writer.write(title);
        writer.write("\n\n");
        if (paths.empty())
            {
            writer.write(emptyMessage);
            writer.write("\n");
            return { core::ExitCode::success, "" };
            }

        for (const auto& path : paths)
            {
            const std::filesystem::path absolute = options.inputPath / path;

            core::FileEntry entry;
            entry.relativePath = path;
            std::string content;
            core::RunResult readResult = readContentWithCache(absolute,
                path.string(),
                redactor,
                cache,
                content,
                &entry.tokenCount);
            if (readResult.code != core::ExitCode::success)
                {
                return readResult;
                }

            entry.sizeBytes = static_cast<std::uintmax_t>(content.size());
            entry.fileClass = core::classifyExtension(core::toLowerCopy(entry.relativePath.extension().string()));
            writeChunkMarkerBlock(writer, entry, content, options.markers, options.emitFrontmatter);
            writer.write("\n");
            if (!writer.ok)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            }

        return { core::ExitCode::success, "" };
        }
    }
