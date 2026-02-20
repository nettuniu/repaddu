#include "format_writer_internal.h"

namespace repaddu::format::detail
    {
    std::uintmax_t markerBlockBytes(const core::FileEntry& entry,
        const std::string& content,
        core::MarkerMode mode,
        bool emitFrontmatter)
        {
        OutputWriter counter;
        writeChunkMarkerBlock(counter, entry, content, mode, emitFrontmatter);
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
                core::RunResult readResult = readContentWithCache(entry.absolutePath,
                    entry.relativePath.string(),
                    redactor,
                    cache,
                    content,
                    &outTokenCounts[fileIndex]);
                if (readResult.code != core::ExitCode::success)
                    {
                    return readResult;
                    }
                entry.tokenCount = outTokenCounts[fileIndex];

                std::uintmax_t blockBytes = markerBlockBytes(entry, content, options.markers,
                    options.emitFrontmatter);
                blockBytes += 1;

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
