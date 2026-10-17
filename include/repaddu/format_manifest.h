#ifndef REPADDU_FORMAT_MANIFEST_H
#define REPADDU_FORMAT_MANIFEST_H

#include "repaddu/core_types.h"
#include "repaddu/noise_detector.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace repaddu::format
    {
    struct ManifestFileEntry
        {
        std::string path;
        std::string sha256;
        std::uintmax_t sizeBytes = 0;
        std::uintmax_t tokenCount = 0;
        std::string language;
        std::string fileClass;
        std::string outputChunk;
        };

    struct ManifestOutputEntry
        {
        std::string filename;
        std::uintmax_t sizeBytes = 0;
        };

    struct ManifestStats
        {
        std::size_t totalFiles = 0;
        std::uintmax_t totalBytes = 0;
        std::uintmax_t totalTokens = 0;
        std::size_t totalOutputs = 0;
        };

    struct ContextManifestData
        {
        int schemaVersion = 1;
        std::string generatorName = "repaddu";
        std::string generatorVersion = std::string(core::kRepadduVersion);
        std::string repositoryRoot;
        std::string gitCommit;
        std::string gitBranch;
        ManifestStats stats;
        std::vector<ManifestFileEntry> files;
        std::vector<ManifestOutputEntry> outputs;
        noise::ExclusionReport exclusionReport;
        };

    // @note Builds formatted JSON string for context manifest
    std::string buildManifestJson(const ContextManifestData& data);
    }

#endif // REPADDU_FORMAT_MANIFEST_H
