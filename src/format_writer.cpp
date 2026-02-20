#include "repaddu/format_writer.h"

#include "format_writer_alt_formats.h"
#include "format_writer_internal.h"

#include "repaddu/logger.h"

#include <filesystem>
#include <fstream>
#include <memory>

namespace repaddu::format
    {
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

        std::vector<detail::OutputPlanEntry> outputs;
        std::vector<detail::ChunkPartPlan> chunkParts;
        std::vector<std::uintmax_t> tokenCounts(files.size(), 0);
        detail::ContentCache contentCache;
        const std::string overviewName = detail::padNumber(0, options.numberWidth) + "_overview.md";

        int outputIndex = 1;
        if (options.emitTree)
            {
            detail::OutputWriter counter;
            detail::writeTreeOutput(counter, overviewName, treeListing);
            outputs.push_back({ detail::padNumber(outputIndex, options.numberWidth) + "_tree.md", counter.bytes });
            ++outputIndex;
            }

        if (options.emitCMake)
            {
            detail::OutputWriter counter;
            core::RunResult cmakeResult = detail::writeAggregatedFilesOutput(counter,
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
            outputs.push_back({ detail::padNumber(outputIndex, options.numberWidth) + "_cmake.md", counter.bytes });
            ++outputIndex;
            }

        if (options.emitBuildFiles)
            {
            detail::OutputWriter counter;
            core::RunResult buildFilesResult = detail::writeAggregatedFilesOutput(counter,
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
            outputs.push_back({ detail::padNumber(outputIndex, options.numberWidth) + "_build_context.md", counter.bytes });
            ++outputIndex;
            }

        core::RunResult chunkPlanResult = detail::planChunkOutputs(options,
            overviewName,
            chunks,
            files,
            chunkParts,
            outputs,
            outputIndex,
            redactor.get(),
            tokenCounts,
            &contentCache);
        if (chunkPlanResult.code != core::ExitCode::success)
            {
            return chunkPlanResult;
            }

        if (options.maxFiles > 0 && static_cast<int>(outputs.size() + 1) > options.maxFiles)
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

        const std::string overviewContent = detail::overviewTemplate(overviewName,
            options.markers,
            options.emitBuildFiles,
            options.emitLinks,
            outputNames);
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
            const std::filesystem::path outPath = options.outputPath / (detail::padNumber(1, options.numberWidth) + "_tree.md");
            std::ofstream stream(outPath, std::ios::binary);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            detail::OutputWriter writer{ &stream };
            detail::writeTreeOutput(writer, overviewName, treeListing);
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
            const std::filesystem::path outPath = options.outputPath / (detail::padNumber(fileIndexBase, options.numberWidth) + "_cmake.md");
            std::ofstream stream(outPath, std::ios::binary);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            detail::OutputWriter writer{ &stream };
            core::RunResult cmakeResult = detail::writeAggregatedFilesOutput(writer,
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
            const std::filesystem::path outPath = options.outputPath / (detail::padNumber(fileIndexBase, options.numberWidth) + "_build_context.md");
            std::ofstream stream(outPath, std::ios::binary);
            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed to write output file." };
                }
            detail::OutputWriter writer{ &stream };
            core::RunResult buildFilesResult = detail::writeAggregatedFilesOutput(writer,
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
            detail::OutputWriter writer{ &stream };
            writer.write(detail::boilerplateLine(overviewName));
            writer.write("# ");
            writer.write(part.title);
            writer.write("\n\n");

            for (std::size_t fileIndex : part.fileIndices)
                {
                core::FileEntry entry = files[fileIndex];
                std::string content;
                core::RunResult readResult = detail::readContentWithCache(entry.absolutePath,
                    entry.relativePath.string(),
                    redactor.get(),
                    &contentCache,
                    content,
                    nullptr);
                if (readResult.code != core::ExitCode::success)
                    {
                    return readResult;
                    }
                entry.tokenCount = tokenCounts[fileIndex];
                detail::writeChunkMarkerBlock(writer, entry, content, options.markers, options.emitFrontmatter);
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
