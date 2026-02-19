#include "repaddu/app/analysis_backend.h"

#include "repaddu/analysis_graph.h"

namespace repaddu::app
    {
    core::RunResult DefaultAnalysisBackend::populateGraph(const core::CliOptions&,
        const std::vector<core::FileEntry>&,
        const std::vector<std::size_t>&,
        analysis::AnalysisGraph&)
        {
        // Default backend preserves current behavior: no additional graph extraction.
        return { core::ExitCode::success, "" };
        }
    }
