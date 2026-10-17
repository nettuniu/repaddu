#ifndef REPADDU_NOISE_DETECTOR_H
#define REPADDU_NOISE_DETECTOR_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace repaddu::noise
    {
    enum class NoiseType
        {
        none = 0,
        vendor,
        generated,
        build_artifact,
        cache_or_temp,
        package_lock,
        minified
        };

    enum class ExclusionReason
        {
        binary = 0,
        too_large,
        vendor,
        generated,
        extension_filter,
        hidden,
        noise
        };

    struct ExcludedFileEntry
        {
        std::filesystem::path relativePath;
        ExclusionReason reason = ExclusionReason::extension_filter;
        std::uintmax_t sizeBytes = 0;
        std::string details;
        };

    struct ExclusionReport
        {
        std::vector<ExcludedFileEntry> excludedFiles;
        std::size_t countBinary = 0;
        std::size_t countTooLarge = 0;
        std::size_t countVendor = 0;
        std::size_t countGenerated = 0;
        std::size_t countExtension = 0;
        std::size_t countHidden = 0;
        std::size_t countNoise = 0;

        std::size_t totalExcluded() const noexcept
            {
            return excludedFiles.size();
            }

        void record(const std::filesystem::path& relPath,
            ExclusionReason reason,
            std::uintmax_t size = 0,
            std::string_view details = "");
        };

    // @note Path and content classifiers
    bool isVendorPath(const std::filesystem::path& relativePath);
    bool isNoisePath(const std::filesystem::path& relativePath);
    bool isLockFile(const std::filesystem::path& relativePath);
    bool isMinifiedFile(const std::filesystem::path& relativePath);
    bool isGeneratedSample(std::string_view sampleContent);
    bool isGeneratedCode(const std::filesystem::path& filePath);

    NoiseType classifyNoise(const std::filesystem::path& relativePath, std::string_view sampleContent = "");
    std::string_view noiseTypeLabel(NoiseType type);
    std::string_view exclusionReasonLabel(ExclusionReason reason);
    std::string renderExclusionSummaryText(const ExclusionReport& report);
    }

#endif // REPADDU_NOISE_DETECTOR_H
