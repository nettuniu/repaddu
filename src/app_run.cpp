#include "repaddu/app_run.h"

#include "repaddu/app_analyze.h"
#include "repaddu/config_generator.h"
#include "repaddu/format_language_report.h"
#include "repaddu/format_tree.h"
#include "repaddu/format_writer.h"
#include "repaddu/grouping_component_map.h"
#include "repaddu/grouping_strategies.h"
#include "repaddu/io_traversal.h"
#include "repaddu/language_profiles.h"

#include <iostream>

namespace repaddu::app
    {
    core::RunResult run(const core::CliOptions& options, ui::UserInterface& ui)
        {
        if (options.generateConfig)
            {
            core::RunResult result = config::generateDefaultConfig(options.configPath);
            ui.logInfo(result.message);
            return result;
            }

        io::TraversalResult traversal;
        ui.startProgress("Scanning repository", 0);
        core::RunResult traversalResult = io::traverseRepository(options, traversal);
        ui.endProgress();

        if (traversalResult.code != core::ExitCode::success)
            {
            return traversalResult;
            }

        if (options.scanLanguages)
            {
            const std::string report = format::renderLanguageReport(options, traversal.files);
            std::cout << report;
            return { core::ExitCode::success, "" };
            }

        core::CliOptions effectiveOptions = options;
        if (effectiveOptions.language.empty() || effectiveOptions.buildSystem.empty())
            {
            const core::DetectionResult detected = core::detectLanguageAndBuildSystem(traversal.files);
            if (effectiveOptions.language.empty())
                {
                effectiveOptions.language = detected.languageId;
                }
            if (effectiveOptions.buildSystem.empty())
                {
                effectiveOptions.buildSystem = detected.buildSystemId;
                }
            }

        grouping::ComponentMap componentMap;
        const grouping::ComponentMap* componentMapPtr = nullptr;
        if (effectiveOptions.groupBy == core::GroupingMode::component)
            {
            core::RunResult mapResult = grouping::loadComponentMap(effectiveOptions.componentMapPath, componentMap);
            if (mapResult.code != core::ExitCode::success)
                {
                return mapResult;
                }
            componentMapPtr = &componentMap;
            }

        core::RunResult groupingResult;
        ui.startProgress("Grouping files", 0);
        grouping::GroupingResult grouped = grouping::filterAndGroupFiles(effectiveOptions, traversal.files, componentMapPtr, groupingResult);
        ui.endProgress();

        if (groupingResult.code != core::ExitCode::success)
            {
            return groupingResult;
            }

        if (options.analyzeOnly)
            {
            std::string report;
            const core::RunResult analyzeResult = buildAnalyzeOnlyReport(effectiveOptions, traversal.files, grouped.includedIndices, report);
            if (analyzeResult.code != core::ExitCode::success)
                {
                return analyzeResult;
                }
            std::cout << report;
            return { core::ExitCode::success, "" };
            }

        core::RunResult chunkResult;
        std::vector<core::OutputChunk> chunks = grouping::chunkGroups(effectiveOptions, traversal.files, grouped.groups, chunkResult);
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

        ui.startProgress("Writing outputs", static_cast<int>(chunks.size()));
        core::RunResult writeResult = format::writeOutputs(effectiveOptions, traversal.files, chunks, treeListing, traversal.cmakeLists, traversal.buildFiles);
        ui.endProgress();

        return writeResult;
        }
    }
