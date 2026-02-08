#include "repaddu/format_analysis_report.h"
#include "repaddu/format_language_report.h"
#include "repaddu/analysis_tokens.h"
#include "repaddu/analysis_view.h"
#include <sstream>
#include <iomanip>
#include <numeric>

namespace repaddu::format
    {
    std::string renderAnalysisReport(const core::CliOptions& options,
                                     const std::vector<core::FileEntry>& allFiles,
                                     const std::vector<std::size_t>& includedIndices)
        {
        std::ostringstream out;
        
        std::uintmax_t totalSize = 0;
        for (const auto& f : allFiles) totalSize += f.sizeBytes;

        std::uintmax_t includedSize = 0;
        std::uintmax_t includedTokens = 0;
        std::vector<core::FileEntry> includedFiles;
        includedFiles.reserve(includedIndices.size());

        for (std::size_t idx : includedIndices)
            {
            const auto& f = allFiles[idx];
            includedSize += f.sizeBytes;
            // Note: f.tokenCount might be 0 if not read yet, 
            // but in analyze mode we should probably estimate them all.
            // However, traverseRepository doesn't read content. 
            // We'll do a quick estimate based on size for the report if 0.
            std::uintmax_t tokens = f.tokenCount;
            if (tokens == 0 && f.sizeBytes > 0 && !f.isBinary)
                {
                tokens = f.sizeBytes / 4; // very rough heuristic for analysis mode
                }
            includedTokens += tokens;
            includedFiles.push_back(f);
            }

        out << "==========================================\n";
        out << "        REPADDU ANALYSIS REPORT           \n";
        out << "==========================================\n\n";
        
        out << "Repository: " << options.inputPath.string() << "\n\n";
        
        out << "OVERALL SCAN:\n";
        out << "  Total files found:    " << allFiles.size() << "\n";
        out << "  Total size:           " << totalSize << " bytes\n\n";
        
        out << "INCLUSION (After filtering):\n";
        out << "  Files included:       " << includedIndices.size() << "\n";
        out << "  Included size:        " << includedSize << " bytes\n";
        out << "  Estimated tokens:     ~" << includedTokens << "\n\n";

        out << "LANGUAGE BREAKDOWN (Included files):\n";
        out << renderLanguageReport(options, includedFiles);
        
        out << "\n==========================================\n";
        
        return out.str();
        }

    std::string renderAnalysisReportWithViews(const core::CliOptions& options,
                                              const std::vector<core::FileEntry>& allFiles,
                                              const std::vector<std::size_t>& includedIndices,
                                              const analysis::AnalysisGraph& graph,
                                              const analysis::AnalysisViewOptions& viewOptions)
        {
        std::ostringstream out;
        out << renderAnalysisReport(options, allFiles, includedIndices);

        if (!options.analysisEnabled)
            {
            return out.str();
            }

        analysis::AnalysisViewRegistry registry = analysis::buildDefaultViewRegistry();
        std::vector<std::string> views = options.analysisViews;
        if (views.empty())
            {
            views = registry.viewNames();
            }

        out << "\nANALYSIS VIEWS\n";
        out << "====================\n";

        for (const auto& viewName : views)
            {
            const analysis::AnalysisViewResult view = registry.render(viewName, graph, viewOptions);
            out << "\nVIEW: " << view.name << "\n";
            if (view.metadata.count("error") != 0)
                {
                out << "error: " << view.metadata.at("error") << "\n";
                continue;
                }

            if (view.nodes.empty())
                {
                out << "nodes: none\n";
                }
            else
                {
                out << "nodes:\n";
                for (const auto& node : view.nodes)
                    {
                    out << "node: " << node.id;
                    if (!node.group.empty())
                        {
                        out << " group=" << node.group;
                        }
                    if (!node.label.empty())
                        {
                        out << " label=" << node.label;
                        }
                    out << "\n";
                    }
                }

            if (view.edges.empty())
                {
                out << "edges: none\n";
                }
            else
                {
                out << "edges:\n";
                for (const auto& edge : view.edges)
                    {
                    out << "edge: " << edge.from << " -> " << edge.to;
                    if (!edge.label.empty())
                        {
                        out << " label=" << edge.label;
                        }
                    out << "\n";
                    }
                }
            }

        return out.str();
        }
    }
