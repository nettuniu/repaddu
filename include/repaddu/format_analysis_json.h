#ifndef REPADDU_FORMAT_ANALYSIS_JSON_H
#define REPADDU_FORMAT_ANALYSIS_JSON_H

#include "repaddu/core_types.h"
#include <string>
#include <vector>

namespace repaddu::analysis
    {
    class AnalysisGraph;
    struct AnalysisViewOptions;
    }

namespace repaddu::format
    {
    std::string renderAnalysisJson(const core::CliOptions& options,
                                   const std::vector<core::FileEntry>& allFiles,
                                   const std::vector<std::size_t>& includedIndices,
                                   const analysis::AnalysisGraph* graph = nullptr,
                                   const analysis::AnalysisViewOptions* viewOptions = nullptr);
    }

#endif // REPADDU_FORMAT_ANALYSIS_JSON_H
