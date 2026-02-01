#include "repaddu/cli_run.h"

#include "repaddu/core_types.h"
#include "repaddu/format_language_report.h"
#include "repaddu/format_tree.h"
#include "repaddu/format_writer.h"
#include "repaddu/grouping_component_map.h"
#include "repaddu/grouping_strategies.h"
#include "repaddu/io_traversal.h"

#include <iostream>

namespace repaddu::cli
    {
    core::RunResult run(const core::CliOptions& options)
        {
        io::TraversalResult traversal;
        core::RunResult traversalResult = io::traverseRepository(options, traversal);
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
        grouping::GroupingResult grouped = grouping::filterAndGroupFiles(options, traversal.files, componentMapPtr, groupingResult);
        if (groupingResult.code != core::ExitCode::success)
            {
            return groupingResult;
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

        return format::writeOutputs(options, traversal.files, chunks, treeListing, traversal.cmakeLists, traversal.buildFiles);
        }
    }
