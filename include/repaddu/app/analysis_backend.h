#ifndef REPADDU_APP_ANALYSIS_BACKEND_H
#define REPADDU_APP_ANALYSIS_BACKEND_H

#include "repaddu/core_types.h"

#include <cstddef>
#include <vector>

namespace repaddu::analysis
    {
    class AnalysisGraph;
    }

namespace repaddu::app
    {
    class AnalysisBackend
        {
        public:
            virtual ~AnalysisBackend() = default;

            virtual core::RunResult populateGraph(const core::CliOptions& options,
                const std::vector<core::FileEntry>& files,
                const std::vector<std::size_t>& includedIndices,
                analysis::AnalysisGraph& graph) = 0;
        };

    class DefaultAnalysisBackend : public AnalysisBackend
        {
        public:
            core::RunResult populateGraph(const core::CliOptions& options,
                const std::vector<core::FileEntry>& files,
                const std::vector<std::size_t>& includedIndices,
                analysis::AnalysisGraph& graph) override;
        };
    }

#endif // REPADDU_APP_ANALYSIS_BACKEND_H
