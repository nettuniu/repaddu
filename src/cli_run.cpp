#include "repaddu/cli_run.h"

#include "repaddu/core_types.h"
#include "repaddu/config_generator.h"
#include "repaddu/format_language_report.h"
#include "repaddu/format_analysis_report.h"
#include "repaddu/format_analysis_json.h"
#include "repaddu/analysis_graph.h"
#include "repaddu/analysis_view.h"
#include "repaddu/analysis_tags.h"
#include "repaddu/format_tree.h"
#include "repaddu/format_writer.h"
#include "repaddu/grouping_component_map.h"
#include "repaddu/grouping_strategies.h"
#include "repaddu/io_traversal.h"
#include "repaddu/language_profiles.h"
#include "repaddu/ui_console.h"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>

namespace repaddu::cli
    {
    namespace
        {
        std::string readFileText(const std::filesystem::path& path)
            {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
                {
                return {};
                }
            std::ostringstream buffer;
            buffer << stream.rdbuf();
            return buffer.str();
            }

        std::string renderTagSummary(const core::CliOptions& options,
            const std::vector<core::FileEntry>& files,
            const std::vector<std::size_t>& includedIndices)
            {
            analysis::TagExtractor extractor;
            if (!options.tagPatternsPath.empty())
                {
                extractor.loadTagPatternsFromFile(options.tagPatternsPath, false);
                }

            std::map<std::string, std::size_t> counts;
            std::vector<analysis::TagMatch> matches;
            for (std::size_t index : includedIndices)
                {
                const auto& entry = files[index];
                if (entry.isBinary)
                    {
                    continue;
                    }
                const std::string content = readFileText(entry.absolutePath);
                if (content.empty())
                    {
                    continue;
                    }
                const auto fileMatches = extractor.extract(content, entry.relativePath.string());
                for (const auto& match : fileMatches)
                    {
                    counts[match.tag] += 1;
                    matches.push_back(match);
                    }
                }

            std::ostringstream out;
            out << "\nTAG SUMMARY\n";
            out << "===========\n";
            if (matches.empty())
                {
                out << "No matching tags found.\n";
                return out.str();
                }

            out << "Counts:\n";
            for (const auto& entry : counts)
                {
                out << "- " << entry.first << ": " << entry.second << "\n";
                }

            out << "\nMatches:\n";
            for (const auto& match : matches)
                {
                out << "- " << match.filePath << ":" << match.lineNumber
                    << " [" << match.tag << "] " << match.content << "\n";
                }
            return out.str();
            }
        }

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
            core::RunResult result = config::generateDefaultConfig(options.configPath);
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
        ui->startProgress("Grouping files", 0);
        grouping::GroupingResult grouped = grouping::filterAndGroupFiles(effectiveOptions, traversal.files, componentMapPtr, groupingResult);
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
            viewOptions.collapseMode = effectiveOptions.analysisCollapse;
            if (effectiveOptions.format == core::OutputFormat::jsonl)
                {
                report = format::renderAnalysisJson(effectiveOptions, traversal.files, grouped.includedIndices,
                    &graph, &viewOptions);
                }
            else
                {
                report = format::renderAnalysisReportWithViews(effectiveOptions, traversal.files, grouped.includedIndices, graph, viewOptions);
                }
            if (effectiveOptions.extractTags)
                {
                report += renderTagSummary(effectiveOptions, traversal.files, grouped.includedIndices);
                }
            std::cout << report; // Report goes to stdout
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

        ui->startProgress("Writing outputs", static_cast<int>(chunks.size()));
        core::RunResult writeResult = format::writeOutputs(effectiveOptions, traversal.files, chunks, treeListing, traversal.cmakeLists, traversal.buildFiles);
        ui->endProgress();
        
        return writeResult;
        }
    }
