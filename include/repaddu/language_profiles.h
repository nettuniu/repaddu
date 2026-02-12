#ifndef REPADDU_LANGUAGE_PROFILES_H
#define REPADDU_LANGUAGE_PROFILES_H

#include "repaddu/core_types.h"

#include <string>
#include <string_view>
#include <vector>

namespace repaddu::core
    {
    struct LanguageProfile
        {
        std::string id;
        std::string displayName;
        std::vector<std::string> sourceExtensions;
        std::vector<std::string> headerExtensions;
        std::vector<std::string> buildFiles;
        bool supportsHeaders = false;
        };

    struct BuildSystemProfile
        {
        std::string id;
        std::vector<std::string> buildFiles;
        };

    struct DetectionResult
        {
        std::string languageId;
        std::string buildSystemId;
        };

    const LanguageProfile* findLanguageProfile(std::string_view id);
    const BuildSystemProfile* findBuildSystemProfile(std::string_view id);
    DetectionResult detectLanguageAndBuildSystem(const std::vector<FileEntry>& files);
    std::vector<std::string> resolveBuildFileNames(const CliOptions& options);
    }

#endif // REPADDU_LANGUAGE_PROFILES_H
