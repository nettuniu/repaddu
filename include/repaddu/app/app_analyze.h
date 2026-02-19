#ifndef REPADDU_APP_ANALYZE_H
#define REPADDU_APP_ANALYZE_H

#include "repaddu/core_types.h"

#include <string>
#include <vector>

namespace repaddu::app
    {
    class AnalysisBackend;

    core::RunResult buildAnalyzeOnlyReport(const core::CliOptions& effectiveOptions,
        const std::vector<core::FileEntry>& files,
        const std::vector<std::size_t>& includedIndices,
        std::string& outReport);

    core::RunResult buildAnalyzeOnlyReport(const core::CliOptions& effectiveOptions,
        const std::vector<core::FileEntry>& files,
        const std::vector<std::size_t>& includedIndices,
        AnalysisBackend& backend,
        std::string& outReport);
    }

#endif // REPADDU_APP_ANALYZE_H
