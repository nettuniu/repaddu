#ifndef REPADDU_ANALYSIS_CPP_H
#define REPADDU_ANALYSIS_CPP_H

#include "repaddu/analysis_graph.h"
#include "repaddu/core_types.h"

#include <filesystem>

namespace repaddu::analysis
    {
    struct CppAnalysisOptions
        {
        std::filesystem::path compileCommandsPath;
        bool deep = false;
        };

    core::RunResult analyzeCppProject(const CppAnalysisOptions& options, AnalysisGraph& graph);
    }

#endif // REPADDU_ANALYSIS_CPP_H
