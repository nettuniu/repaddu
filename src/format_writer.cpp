#include "repaddu/format_writer.h"
#include "repaddu/pii_redactor.h"
#include "repaddu/analysis_tokens.h"
#include "repaddu/logger.h"
#include "format_writer_alt_formats.h"

#include "repaddu/core_types.h"

#include <limits>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace repaddu::format
    {
    namespace detail
        {
        namespace
            {
            bool tryReadFileMmap(const std::filesystem::path& path, std::string& outContent)
                {
#if defined(_WIN32)
                const std::wstring widePath = path.wstring();
                HANDLE fileHandle = CreateFileW(widePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (fileHandle == INVALID_HANDLE_VALUE)
                    {
                    return false;
                    }

                LARGE_INTEGER size;
                if (!GetFileSizeEx(fileHandle, &size))
                    {
                    CloseHandle(fileHandle);
                    return false;
                    }

                if (size.QuadPart <= 0)
                    {
                    outContent.clear();
                    CloseHandle(fileHandle);
                    return true;
                    }

                if (size.QuadPart > static_cast<LONGLONG>((std::numeric_limits<std::size_t>::max)()))
                    {
                    CloseHandle(fileHandle);
                    return false;
                    }

                HANDLE mapping = CreateFileMappingW(fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
                if (!mapping)
                    {
                    CloseHandle(fileHandle);
                    return false;
                    }

                void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
                if (!view)
                    {
                    CloseHandle(mapping);
                    CloseHandle(fileHandle);
                    return false;
                    }

                const std::size_t sizeBytes = static_cast<std::size_t>(size.QuadPart);
                const char* data = static_cast<const char*>(view);
                outContent.assign(data, data + sizeBytes);

                UnmapViewOfFile(view);
                CloseHandle(mapping);
                CloseHandle(fileHandle);
                return true;
#else
                const int fd = ::open(path.c_str(), O_RDONLY);
                if (fd < 0)
                    {
                    return false;
                    }

                struct stat statbuf;
                if (::fstat(fd, &statbuf) != 0)
                    {
                    ::close(fd);
                    return false;
                    }

                if (statbuf.st_size <= 0)
                    {
                    outContent.clear();
                    ::close(fd);
                    return true;
                    }

                if (static_cast<unsigned long long>(statbuf.st_size) > std::numeric_limits<std::size_t>::max())
                    {
                    ::close(fd);
                    return false;
                    }

                void* mapping = ::mmap(nullptr, static_cast<std::size_t>(statbuf.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
                if (mapping == MAP_FAILED)
                    {
                    ::close(fd);
                    return false;
                    }

                const std::size_t sizeBytes = static_cast<std::size_t>(statbuf.st_size);
                const char* data = static_cast<const char*>(mapping);
                outContent.assign(data, data + sizeBytes);

                ::munmap(mapping, sizeBytes);
                ::close(fd);
                return true;
#endif
                }
            }

        std::string readFileContent(const std::filesystem::path& path,
            core::RunResult& outResult,
            std::uintmax_t* outTokens,
            security::PiiRedactor* redactor,
            const std::string& relativePath)
            {
            std::string content;
            if (!tryReadFileMmap(path, content))
                {
                std::ifstream stream(path, std::ios::binary);
                if (!stream)
                    {
                    outResult = { core::ExitCode::io_failure, "Failed to open file for reading." };
                    return {};
                    }
                std::ostringstream buffer;
                buffer << stream.rdbuf();
                content = buffer.str();
                }

            outResult = { core::ExitCode::success, "" };

            if (redactor)
                {
                content = redactor->redact(content, relativePath);
                }

            if (outTokens)
                {
                *outTokens = analysis::TokenEstimator::estimateTokens(content);
                }

            return content;
            }
        }

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

        struct OutputPlanEntry
            {
            std::string filename;
            std::uintmax_t contentBytes = 0;
            };

        struct ChunkPartPlan
            {
            std::string filename;
            std::string category;
            std::string title;
            std::vector<std::size_t> fileIndices;
            std::uintmax_t contentBytes = 0;
            };

        struct OutputWriter
            {
            std::ostream* stream = nullptr;
            std::uintmax_t bytes = 0;
            bool ok = true;

            void write(std::string_view text)
                {
                bytes += static_cast<std::uintmax_t>(text.size());
                if (!stream)
                    {
                    return;
                    }
                (*stream) << text;
                if (!(*stream))
                    {
                    ok = false;
                    }
                }
            };

        struct ContentCache
            {
            std::unordered_map<std::string, std::string> entries;
            std::uintmax_t totalBytes = 0;
            std::uintmax_t maxBytes = 4 * 1024 * 1024;

            bool tryGet(const std::filesystem::path& path, std::string& outContent) const
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

            void store(const std::filesystem::path& path, const std::string& content)
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
            };

        void writeMarkerBlock(OutputWriter& writer, const core::FileEntry& entry, const std::string& content,
            core::MarkerMode mode, bool emitFrontmatter)
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

        std::string overviewTemplate(const std::string& overviewName, core::MarkerMode mode, bool emitBuildFiles,
            bool emitLinks, const std::vector<std::string>& generatedFiles)
            {
            std::ostringstream out;
            out << "# repaddu output specification\n\n";
            out << "This file explains the layout and markers used in every other output file.\n";
            out << "All other files start with a short note: \"Read \"" << overviewName
                << "\" first for format and conventions.\"\n\n";

            out << "## Table of Contents\n";
            for (const auto& filename : generatedFiles)
                {
                // Skip self referencing link if desired, or include it.
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
            std::uintmax_t* outTokens = nullptr)
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
            outContent = detail::readFileContent(path, readResult, outTokens, redactor, relativePath);
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
                core::RunResult readResult = readContentWithCache(absolute, path.string(), redactor, cache, content, &entry.tokenCount);
                if (readResult.code != core::ExitCode::success)
                    {
                    return readResult;
                    }

                entry.sizeBytes = static_cast<std::uintmax_t>(content.size());
                entry.fileClass = core::classifyExtension(core::toLowerCopy(entry.relativePath.extension().string()));
                writeMarkerBlock(writer, entry, content, options.markers, options.emitFrontmatter);
                writer.write("\n");
                if (!writer.ok)
                    {
                    return { core::ExitCode::io_failure, "Failed to write output file." };
                    }
                }

            return { core::ExitCode::success, "" };
            }

        std::uintmax_t markerBlockBytes(const core::FileEntry& entry, const std::string& content,
            core::MarkerMode mode, bool emitFrontmatter)
            {
            OutputWriter counter;
            writeMarkerBlock(counter, entry, content, mode, emitFrontmatter);
            return counter.bytes;
            }


        core::RunResult planChunkOutputs(const core::CliOptions& options,
            const std::string& overviewName,
            const std::vector<core::OutputChunk>& chunks,
            const std::vector<core::FileEntry>& files,
            std::vector<ChunkPartPlan>& outParts,
            std::vector<OutputPlanEntry>& outOutputs,
            int& index,
            security::PiiRedactor* redactor,
            std::vector<std::uintmax_t>& outTokenCounts,
            ContentCache* cache)
            {
            for (const auto& chunk : chunks)
                {
                std::string header = boilerplateLine(overviewName);
                header += "# " + chunk.title + "\n\n";
                const std::uintmax_t headerBytes = static_cast<std::uintmax_t>(header.size());
                std::uintmax_t currentBytes = headerBytes;
                int part = 1;
                std::vector<std::size_t> currentIndices;

                for (std::size_t fileIndex : chunk.fileIndices)
                    {
                    core::FileEntry entry = files[fileIndex];
                    std::string content;
                    core::RunResult readResult = readContentWithCache(entry.absolutePath, entry.relativePath.string(), redactor, cache, content, &outTokenCounts[fileIndex]);
                    if (readResult.code != core::ExitCode::success)
                        {
                        return readResult;
                        }
                    entry.tokenCount = outTokenCounts[fileIndex];

                    std::uintmax_t blockBytes = markerBlockBytes(entry, content, options.markers,
                        options.emitFrontmatter);
                    blockBytes += 1; // newline between blocks

                    if (options.maxBytes > 0 && !chunk.fileIndices.empty()
                        && currentBytes + blockBytes > options.maxBytes)
                        {
                        ChunkPartPlan partPlan;
                        const std::string partSuffix = (part > 1) ? "_part" + std::to_string(part) : "";
                        partPlan.filename = padNumber(index, options.numberWidth) + "_" + chunk.category + partSuffix + ".md";
                        partPlan.category = chunk.category;
                        partPlan.title = chunk.title;
                        partPlan.fileIndices = std::move(currentIndices);
                        partPlan.contentBytes = currentBytes;
                        outParts.push_back(std::move(partPlan));
                        outOutputs.push_back({ outParts.back().filename, outParts.back().contentBytes });

                        ++index;
                        ++part;
                        currentIndices.clear();
                        currentBytes = headerBytes;
                        }

                    currentIndices.push_back(fileIndex);
                    currentBytes += blockBytes;

                    if (options.maxBytes > 0 && blockBytes > options.maxBytes)
                        {
                        return { core::ExitCode::output_constraints, "A single file block exceeds --max-bytes." };
                        }
                    }

                if (currentBytes > 0)
                    {
                    ChunkPartPlan partPlan;
                    const std::string partSuffix = (part > 1) ? "_part" + std::to_string(part) : "";
                    partPlan.filename = padNumber(index, options.numberWidth) + "_" + chunk.category + partSuffix + ".md";
                    partPlan.category = chunk.category;
                    partPlan.title = chunk.title;
                    partPlan.fileIndices = std::move(currentIndices);
                    partPlan.contentBytes = currentBytes;
                    outParts.push_back(std::move(partPlan));
                    outOutputs.push_back({ outParts.back().filename, outParts.back().contentBytes });
                    ++index;
                    }
                }

            return { core::ExitCode::success, "" };
            }
        }

    core::RunResult writeOutputs(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const std::vector<core::OutputChunk>& chunks,
        const std::string& treeListing,
        const std::vector<std::filesystem::path>& cmakeLists,
        const std::vector<std::filesystem::path>& buildFiles)
        {
        std::error_code errorCode;
        std::filesystem::create_directories(options.outputPath, errorCode);
        if (errorCode)
            {
            return { core::ExitCode::io_failure, "Failed to create output directory." };
            }

        // Initialize Redactor if needed
        std::unique_ptr<security::PiiRedactor> redactor;
        if (options.redactPii)
            {
            redactor = std::make_unique<security::PiiRedactor>();
            }

        if (options.format == core::OutputFormat::jsonl)
            {
            return detail::writeJsonlOutput(options, files, chunks, redactor.get());
            }

        if (options.format == core::OutputFormat::html)
            {
            return detail::writeHtmlOutput(options, files, redactor.get());
            }

        std::vector<OutputPlanEntry> outputs;
        std::vector<ChunkPartPlan> chunkParts;
        std::vector<std::uintmax_t> tokenCounts(files.size(), 0);
        ContentCache contentCache;
        const std::string overviewName = padNumber(0, options.numberWidth) + "_overview.md";

        int index = 1;
        if (options.emitTree)
            {
            OutputWriter counter;
            writeTreeOutput(counter, overviewName, treeListing);
            outputs.push_back({ padNumber(index, options.numberWidth) + "_tree.md", counter.bytes });
            ++index;
            }

        if (options.emitCMake)
            {
            OutputWriter counter;
            core::RunResult cmakeResult = writeAggregatedFilesOutput(counter,
                overviewName,
                "# Aggregated CMakeLists.txt files",
                "No CMakeLists.txt files were found.",
                cmakeLists,
                options,
                redactor.get(),
                &contentCache);
            if (cmakeResult.code != core::ExitCode::success)
                {
                return cmakeResult;
                }
            outputs.push_back({ padNumber(index, options.numberWidth) + "_cmake.md", counter.bytes });
            ++index;
            }

        if (options.emitBuildFiles)
            {
            OutputWriter counter;
            core::RunResult buildFilesResult = writeAggregatedFilesOutput(counter,
                overviewName,
                "# Aggregated build-system files",
                "No build-system files were found.",
                buildFiles,
                options,
                redactor.get(),
                &contentCache);
            if (buildFilesResult.code != core::ExitCode::success)
                {
                return buildFilesResult;
                }
            outputs.push_back({ padNumber(index, options.numberWidth) + "_build_context.md", counter.bytes });
            ++index;
            }

        core::RunResult chunkPlanResult = planChunkOutputs(options, overviewName, chunks, files, chunkParts, outputs, index, redactor.get(), tokenCounts, &contentCache);
        if (chunkPlanResult.code != core::ExitCode::success)
            {
            return chunkPlanResult;
            }

        if (options.maxFiles > 0 && static_cast<int>(outputs.size() + 1) > options.maxFiles) // +1 for overview
            {
            return { core::ExitCode::output_constraints, "Output file count exceeds --max-files." };
            }

        for (const auto& output : outputs)
            {
            if (options.maxBytes > 0 && output.contentBytes > options.maxBytes)
                {
                return { core::ExitCode::output_constraints, "Output file exceeds --max-bytes." };
                }
            }

        std::vector<std::string> outputNames;
        outputNames.reserve(outputs.size());
        for (const auto& output : outputs)
            {
            outputNames.push_back(output.filename);
            }

        const std::string overviewContent = overviewTemplate(overviewName, options.markers, options.emitBuildFiles,
            options.emitLinks, outputNames);
        const std::uintmax_t overviewBytes = static_cast<std::uintmax_t>(overviewContent.size());
        if (options.maxBytes > 0 && overviewBytes > options.maxBytes)
            {
            return { core::ExitCode::output_constraints, "Output file exceeds --max-bytes." };
            }

        if (options.dryRun)
            {
            LogInfo("[Dry Run] Would write: " + overviewName + " (" + std::to_string(overviewBytes) + " bytes)");
            for (const auto& output : outputs)
                {
                LogInfo("[Dry Run] Would write: " + output.filename + " (" + std::to_string(output.contentBytes) + " bytes)");
                }
            LogInfo("[Dry Run] Simulation complete. No files were written.");
            return { core::ExitCode::success, "" };
            }

        {
            const std::filesystem::path outPath = options.outputPath / overviewName;
            std::ofstream stream(outPath, std::ios::binary);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            stream << overviewContent;
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to flush output file." };
                }
        }

        if (options.emitTree)
            {
            const std::filesystem::path outPath = options.outputPath / (padNumber(1, options.numberWidth) + "_tree.md");
            std::ofstream stream(outPath, std::ios::binary);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            OutputWriter writer{ &stream };
            writeTreeOutput(writer, overviewName, treeListing);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to flush output file." };
                }
            }

        int fileIndexBase = 1;
        if (options.emitTree)
            {
            ++fileIndexBase;
            }

        if (options.emitCMake)
            {
            const std::filesystem::path outPath = options.outputPath / (padNumber(fileIndexBase, options.numberWidth) + "_cmake.md");
            std::ofstream stream(outPath, std::ios::binary);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            OutputWriter writer{ &stream };
            core::RunResult cmakeResult = writeAggregatedFilesOutput(writer,
                overviewName,
                "# Aggregated CMakeLists.txt files",
                "No CMakeLists.txt files were found.",
                cmakeLists,
                options,
                redactor.get(),
                &contentCache);
            if (cmakeResult.code != core::ExitCode::success)
                {
                return cmakeResult;
                }
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to flush output file." };
                }
            ++fileIndexBase;
            }

        if (options.emitBuildFiles)
            {
            const std::filesystem::path outPath = options.outputPath / (padNumber(fileIndexBase, options.numberWidth) + "_build_context.md");
            std::ofstream stream(outPath, std::ios::binary);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            OutputWriter writer{ &stream };
            core::RunResult buildFilesResult = writeAggregatedFilesOutput(writer,
                overviewName,
                "# Aggregated build-system files",
                "No build-system files were found.",
                buildFiles,
                options,
                redactor.get(),
                &contentCache);
            if (buildFilesResult.code != core::ExitCode::success)
                {
                return buildFilesResult;
                }
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to flush output file." };
                }
            ++fileIndexBase;
            }

        for (const auto& part : chunkParts)
            {
            const std::filesystem::path outPath = options.outputPath / part.filename;
            std::ofstream stream(outPath, std::ios::binary);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            OutputWriter writer{ &stream };
            writer.write(boilerplateLine(overviewName));
            writer.write("# ");
            writer.write(part.title);
            writer.write("\n\n");

            for (std::size_t fileIndex : part.fileIndices)
                {
                core::FileEntry entry = files[fileIndex];
                std::string content;
                core::RunResult readResult = readContentWithCache(entry.absolutePath, entry.relativePath.string(), redactor.get(), &contentCache, content, nullptr);
                if (readResult.code != core::ExitCode::success)
                    {
                    return readResult;
                    }
                entry.tokenCount = tokenCounts[fileIndex];
                writeMarkerBlock(writer, entry, content, options.markers, options.emitFrontmatter);
                writer.write("\n");
                if (!writer.ok)
                    {
                    return { core::ExitCode::io_failure, "Failed to write output file." };
                    }
                }

            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to flush output file." };
                }
            }

        return { core::ExitCode::success, "" };
        }
    }
