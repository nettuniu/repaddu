#ifndef REPADDU_FORMAT_ANALYSIS_TAGS_H
#define REPADDU_FORMAT_ANALYSIS_TAGS_H

#include "repaddu/core_types.h"

#include <string>
#include <vector>

namespace repaddu::format
    {
    std::string renderTagSummaryReport(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const std::vector<std::size_t>& includedIndices);
    }

#endif // REPADDU_FORMAT_ANALYSIS_TAGS_H
