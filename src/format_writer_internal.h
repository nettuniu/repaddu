#ifndef REPADDU_FORMAT_WRITER_INTERNAL_H
#define REPADDU_FORMAT_WRITER_INTERNAL_H

#include "repaddu/core_types.h"
#include "repaddu/pii_redactor.h"

#include <filesystem>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace repaddu::format::detail
    {
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

        bool tryGet(const std::filesystem::path& path, std::string& outContent) const;
        void store(const std::filesystem::path& path, const std::string& content);
        };

    std::string padNumber(int value, int width);
    std::string overviewTemplate(const std::string& overviewName,
        core::MarkerMode mode,
        bool emitBuildFiles,
        bool emitLinks,
        const std::vector<std::string>& generatedFiles);
    std::string boilerplateLine(const std::string& overviewName);

    core::RunResult readContentWithCache(const std::filesystem::path& path,
        const std::string& relativePath,
        security::PiiRedactor* redactor,
        ContentCache* cache,
        std::string& outContent,
        std::uintmax_t* outTokens = nullptr);

    void writeChunkMarkerBlock(OutputWriter& writer,
        const core::FileEntry& entry,
        const std::string& content,
        core::MarkerMode mode,
        bool emitFrontmatter);

    void writeTreeOutput(OutputWriter& writer,
        const std::string& overviewName,
        const std::string& treeListing);

    core::RunResult writeAggregatedFilesOutput(OutputWriter& writer,
        const std::string& overviewName,
        const std::string& title,
        const std::string& emptyMessage,
        const std::vector<std::filesystem::path>& paths,
        const core::CliOptions& options,
        security::PiiRedactor* redactor,
        ContentCache* cache);

    std::uintmax_t markerBlockBytes(const core::FileEntry& entry,
        const std::string& content,
        core::MarkerMode mode,
        bool emitFrontmatter);

    core::RunResult planChunkOutputs(const core::CliOptions& options,
        const std::string& overviewName,
        const std::vector<core::OutputChunk>& chunks,
        const std::vector<core::FileEntry>& files,
        std::vector<ChunkPartPlan>& outParts,
        std::vector<OutputPlanEntry>& outOutputs,
        int& index,
        security::PiiRedactor* redactor,
        std::vector<std::uintmax_t>& outTokenCounts,
        ContentCache* cache);
    }

#endif // REPADDU_FORMAT_WRITER_INTERNAL_H
