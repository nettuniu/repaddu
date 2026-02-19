#include "repaddu/app/app_analyze.h"

#include "repaddu/analysis_graph.h"
#include "repaddu/analysis_view.h"
#include "repaddu/format_analysis_json.h"
#include "repaddu/format_analysis_report.h"
#include "repaddu/format/analysis_tags_report.h"

namespace repaddu::app
    {
    core::RunResult buildAnalyzeOnlyReport(const core::CliOptions& effectiveOptions,
        const std::vector<core::FileEntry>& files,
        const std::vector<std::size_t>& includedIndices,
        std::string& outReport)
        {
        outReport.clear();

        analysis::AnalysisGraph graph;
        analysis::AnalysisViewOptions viewOptions;
        viewOptions.collapseMode = effectiveOptions.analysisCollapse;

        if (effectiveOptions.format == core::OutputFormat::jsonl)
            {
            outReport = format::renderAnalysisJson(effectiveOptions, files, includedIndices,
                &graph, &viewOptions);
            }
        else
            {
            outReport = format::renderAnalysisReportWithViews(effectiveOptions, files, includedIndices, graph, viewOptions);
            }

        if (effectiveOptions.extractTags)
            {
            outReport += format::renderTagSummaryReport(effectiveOptions, files, includedIndices);
            }

        return { core::ExitCode::success, "" };
        }
    }
