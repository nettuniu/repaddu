#ifndef REPADDU_ANALYSIS_CPP_H
#define REPADDU_ANALYSIS_CPP_H

#include <filesystem>

namespace repaddu::analysis
    {
    class AnalysisGraph;
    }

namespace repaddu::core
    {
    struct RunResult;
    }

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
