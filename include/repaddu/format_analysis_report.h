#ifndef REPADDU_FORMAT_ANALYSIS_REPORT_H
#define REPADDU_FORMAT_ANALYSIS_REPORT_H

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
    std::string renderAnalysisReport(const core::CliOptions& options,
                                     const std::vector<core::FileEntry>& allFiles,
                                     const std::vector<std::size_t>& includedIndices);

    std::string renderAnalysisReportWithViews(const core::CliOptions& options,
                                              const std::vector<core::FileEntry>& allFiles,
                                              const std::vector<std::size_t>& includedIndices,
                                              const analysis::AnalysisGraph& graph,
                                              const analysis::AnalysisViewOptions& viewOptions);
    }

#endif // REPADDU_FORMAT_ANALYSIS_REPORT_H
