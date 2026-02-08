#ifndef REPADDU_FORMAT_ANALYSIS_JSON_H
#define REPADDU_FORMAT_ANALYSIS_JSON_H

#include "repaddu/core_types.h"
#include <string>
#include <vector>

namespace repaddu::format
    {
    std::string renderAnalysisJson(const core::CliOptions& options,
                                   const std::vector<core::FileEntry>& allFiles,
                                   const std::vector<std::size_t>& includedIndices);
    }

#endif // REPADDU_FORMAT_ANALYSIS_JSON_H
