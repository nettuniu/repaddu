#include "repaddu/cli_run.h"

#include "repaddu/core_types.h"
#include "repaddu/config_generator.h"
#include "repaddu/format_language_report.h"
#include "repaddu/format_analysis_report.h"
#include "repaddu/format_analysis_json.h"
#include "repaddu/analysis_graph.h"
#include "repaddu/analysis_view.h"
#include "repaddu/format_tree.h"
#include "repaddu/format_writer.h"
#include "repaddu/grouping_component_map.h"
#include "repaddu/grouping_strategies.h"
#include "repaddu/io_traversal.h"
#include "repaddu/ui_console.h"

#include <iostream>

namespace repaddu::cli
    {
    core::RunResult run(const core::CliOptions& options, ui::UserInterface* ui)
        {
        // Use provided UI or fallback to local ConsoleUI
        ui::ConsoleUI defaultUI;
        if (!ui)
            {
            ui = &defaultUI;
            }

        if (options.generateConfig)
            {
            core::RunResult result = config::generateDefaultConfig();
            ui->logInfo(result.message);
            return result;
            }

        io::TraversalResult traversal;
        ui->startProgress("Scanning repository", 0);
        core::RunResult traversalResult = io::traverseRepository(options, traversal);
        ui->endProgress();

        if (traversalResult.code != core::ExitCode::success)
            {
            return traversalResult;
            }

        if (options.scanLanguages)
            {
            const std::string report = format::renderLanguageReport(options, traversal.files);
            std::cout << report; // Keep report on stdout as it's the main output
            return { core::ExitCode::success, "" };
            }

        grouping::ComponentMap componentMap;
        const grouping::ComponentMap* componentMapPtr = nullptr;
        if (options.groupBy == core::GroupingMode::component)
            {
            core::RunResult mapResult = grouping::loadComponentMap(options.componentMapPath, componentMap);
            if (mapResult.code != core::ExitCode::success)
                {
                return mapResult;
                }
            componentMapPtr = &componentMap;
            }

        core::RunResult groupingResult;
        ui->startProgress("Grouping files", 0);
        grouping::GroupingResult grouped = grouping::filterAndGroupFiles(options, traversal.files, componentMapPtr, groupingResult);
        ui->endProgress();

        if (groupingResult.code != core::ExitCode::success)
            {
            return groupingResult;
            }

        if (options.analyzeOnly)
            {
            std::string report;
            analysis::AnalysisGraph graph;
            analysis::AnalysisViewOptions viewOptions;
            viewOptions.collapseMode = options.analysisCollapse;
            if (options.format == core::OutputFormat::jsonl)
                {
                report = format::renderAnalysisJson(options, traversal.files, grouped.includedIndices,
                    &graph, &viewOptions);
                }
            else
                {
                report = format::renderAnalysisReportWithViews(options, traversal.files, grouped.includedIndices, graph, viewOptions);
                }
            std::cout << report; // Report goes to stdout
            return { core::ExitCode::success, "" };
            }

        core::RunResult chunkResult;
        std::vector<core::OutputChunk> chunks = grouping::chunkGroups(options, traversal.files, grouped.groups, chunkResult);
        if (chunkResult.code != core::ExitCode::success)
            {
            return chunkResult;
            }

        std::vector<std::filesystem::path> treeFiles;
        treeFiles.reserve(traversal.files.size());
        for (const auto& entry : traversal.files)
            {
            treeFiles.push_back(entry.relativePath);
            }

        const std::string treeListing = format::renderTree(traversal.directories, treeFiles);

        ui->startProgress("Writing outputs", static_cast<int>(chunks.size()));
        core::RunResult writeResult = format::writeOutputs(options, traversal.files, chunks, treeListing, traversal.cmakeLists, traversal.buildFiles);
        ui->endProgress();
        
        return writeResult;
        }
    }
