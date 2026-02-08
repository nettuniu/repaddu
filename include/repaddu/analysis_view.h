#ifndef REPADDU_ANALYSIS_VIEW_H
#define REPADDU_ANALYSIS_VIEW_H

#include "repaddu/analysis_graph.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace repaddu::analysis
    {
    struct ViewNode
        {
        std::string id;
        std::string label;
        std::string group;
        };

    struct ViewEdge
        {
        std::string from;
        std::string to;
        std::string label;
        };

    struct AnalysisViewResult
        {
        std::string name;
        std::vector<ViewNode> nodes;
        std::vector<ViewEdge> edges;
        std::map<std::string, std::string> metadata;
        };

    struct AnalysisViewOptions
        {
        std::string collapseMode = "none";
        };

    using AnalysisViewFunc = std::function<AnalysisViewResult(const AnalysisGraph& graph)>;

    class AnalysisViewRegistry
        {
        public:
            void registerView(const std::string& name, AnalysisViewFunc func);
            bool hasView(const std::string& name) const;
            AnalysisViewResult render(const std::string& name, const AnalysisGraph& graph) const;
            AnalysisViewResult render(const std::string& name, const AnalysisGraph& graph, const AnalysisViewOptions& options) const;
            std::vector<std::string> viewNames() const;

        private:
            std::map<std::string, AnalysisViewFunc> views_;
        };

    AnalysisViewRegistry buildDefaultViewRegistry();
    }

#endif // REPADDU_ANALYSIS_VIEW_H
