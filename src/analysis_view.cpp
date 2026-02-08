#include "repaddu/analysis_view.h"

#include <algorithm>
#include <tuple>

namespace repaddu::analysis
    {
    namespace
        {
        std::string edgeKindLabel(EdgeKind kind)
            {
            switch (kind)
                {
                case EdgeKind::inherits:
                    return "inherits";
                case EdgeKind::overrides:
                    return "overrides";
                case EdgeKind::implemented_by:
                    return "implemented_by";
                }
            return "unknown";
            }

        std::vector<SymbolId> sortedPublicSymbolIds(const AnalysisGraph& graph)
            {
            std::vector<SymbolId> result = graph.publicSymbols();
            std::sort(result.begin(), result.end(),
                [&graph](SymbolId left, SymbolId right)
                {
                return graph.getSymbol(left).qualifiedName < graph.getSymbol(right).qualifiedName;
                });
            return result;
            }

        AnalysisViewResult buildSymbolView(const AnalysisGraph& graph)
            {
            AnalysisViewResult result;
            result.name = "symbols";

            const auto publicIds = sortedPublicSymbolIds(graph);
            result.nodes.reserve(publicIds.size());
            for (SymbolId id : publicIds)
                {
                const auto& symbol = graph.getSymbol(id);
                ViewNode node;
                node.id = symbol.qualifiedName;
                node.label = symbol.name;
                node.group = symbol.containerName;
                result.nodes.push_back(node);
                }

            result.edges.reserve(graph.edges().size());
            for (const auto& edge : graph.edges())
                {
                const auto& from = graph.getSymbol(edge.from);
                const auto& to = graph.getSymbol(edge.to);
                if (!from.isPublic || !to.isPublic)
                    {
                    continue;
                    }
                ViewEdge viewEdge;
                viewEdge.from = from.qualifiedName;
                viewEdge.to = to.qualifiedName;
                viewEdge.label = edgeKindLabel(edge.kind);
                result.edges.push_back(viewEdge);
                }

            std::sort(result.edges.begin(), result.edges.end(),
                [](const ViewEdge& left, const ViewEdge& right)
                {
                if (left.from != right.from)
                    {
                    return left.from < right.from;
                    }
                if (left.to != right.to)
                    {
                    return left.to < right.to;
                    }
                return left.label < right.label;
                });

            return result;
            }

        AnalysisViewResult buildDependencyView(const AnalysisGraph& graph, const AnalysisViewOptions& options)
            {
            AnalysisViewResult result;
            result.name = "dependencies";

            const auto publicIds = sortedPublicSymbolIds(graph);
            for (SymbolId id : publicIds)
                {
                const auto& symbol = graph.getSymbol(id);
                if (symbol.kind == SymbolKind::method_)
                    {
                    continue;
                    }
                ViewNode node;
                node.id = symbol.qualifiedName;
                node.label = symbol.name;
                if (options.collapseMode == "folder" && !symbol.sourcePath.empty())
                    {
                    const std::string path = symbol.sourcePath;
                    const auto lastSlash = path.find_last_of("/\\");
                    node.group = (lastSlash == std::string::npos) ? path : path.substr(0, lastSlash);
                    }
                else if (options.collapseMode == "target" && !symbol.targetName.empty())
                    {
                    node.group = symbol.targetName;
                    }
                else
                    {
                    node.group = symbol.containerName;
                    }
                result.nodes.push_back(node);
                }

            if (options.collapseMode == "none")
                {
                for (const auto& edge : graph.edges())
                    {
                    const auto& from = graph.getSymbol(edge.from);
                    const auto& to = graph.getSymbol(edge.to);
                    if (!from.isPublic || !to.isPublic)
                        {
                        continue;
                        }
                    if (from.kind == SymbolKind::method_ || to.kind == SymbolKind::method_)
                        {
                        continue;
                        }
                    ViewEdge viewEdge;
                    viewEdge.from = from.qualifiedName;
                    viewEdge.to = to.qualifiedName;
                    viewEdge.label = edgeKindLabel(edge.kind);
                    result.edges.push_back(viewEdge);
                    }
                }
            else
                {
                std::map<std::string, std::string> nodeGroups;
                for (const auto& node : result.nodes)
                    {
                    nodeGroups[node.id] = node.group;
                    }
                std::set<std::tuple<std::string, std::string, std::string>> edgeSet;
                for (const auto& edge : graph.edges())
                    {
                    const auto& from = graph.getSymbol(edge.from);
                    const auto& to = graph.getSymbol(edge.to);
                    if (!from.isPublic || !to.isPublic)
                        {
                        continue;
                        }
                    if (from.kind == SymbolKind::method_ || to.kind == SymbolKind::method_)
                        {
                        continue;
                        }
                    const std::string fromGroup = nodeGroups[from.qualifiedName];
                    const std::string toGroup = nodeGroups[to.qualifiedName];
                    if (fromGroup.empty() || toGroup.empty() || fromGroup == toGroup)
                        {
                        continue;
                        }
                    const auto key = std::make_tuple(fromGroup, toGroup, edgeKindLabel(edge.kind));
                    if (edgeSet.insert(key).second)
                        {
                        ViewEdge viewEdge;
                        viewEdge.from = fromGroup;
                        viewEdge.to = toGroup;
                        viewEdge.label = edgeKindLabel(edge.kind);
                        result.edges.push_back(viewEdge);
                        }
                    }

                std::map<std::string, ViewNode> collapsed;
                for (const auto& node : result.nodes)
                    {
                    if (node.group.empty())
                        {
                        continue;
                        }
                    if (collapsed.count(node.group) == 0)
                        {
                        ViewNode collapsedNode;
                        collapsedNode.id = node.group;
                        collapsedNode.label = node.group;
                        collapsedNode.group = "";
                        collapsed[node.group] = collapsedNode;
                        }
                    }
                result.nodes.clear();
                for (const auto& entry : collapsed)
                    {
                    result.nodes.push_back(entry.second);
                    }
                }

            std::sort(result.nodes.begin(), result.nodes.end(),
                [](const ViewNode& left, const ViewNode& right)
                {
                if (left.group != right.group)
                    {
                    return left.group < right.group;
                    }
                return left.id < right.id;
                });

            std::sort(result.edges.begin(), result.edges.end(),
                [](const ViewEdge& left, const ViewEdge& right)
                {
                if (left.from != right.from)
                    {
                    return left.from < right.from;
                    }
                if (left.to != right.to)
                    {
                    return left.to < right.to;
                    }
                return left.label < right.label;
                });

            return result;
            }
        }

    void AnalysisViewRegistry::registerView(const std::string& name, AnalysisViewFunc func)
        {
        views_[name] = std::move(func);
        }

    bool AnalysisViewRegistry::hasView(const std::string& name) const
        {
        return views_.count(name) != 0;
        }

    AnalysisViewResult AnalysisViewRegistry::render(const std::string& name, const AnalysisGraph& graph) const
        {
        AnalysisViewOptions options;
        return render(name, graph, options);
        }

    AnalysisViewResult AnalysisViewRegistry::render(const std::string& name, const AnalysisGraph& graph,
        const AnalysisViewOptions& options) const
        {
        auto it = views_.find(name);
        if (it == views_.end())
            {
            AnalysisViewResult result;
            result.name = name;
            result.metadata["error"] = "unknown view";
            return result;
            }

        if (name == "dependencies")
            {
            return buildDependencyView(graph, options);
            }

        AnalysisViewResult result = it->second(graph);
        return result;
        }
    std::vector<std::string> AnalysisViewRegistry::viewNames() const
        {
        std::vector<std::string> names;
        names.reserve(views_.size());
        for (const auto& entry : views_)
            {
            names.push_back(entry.first);
            }
        return names;
        }

    AnalysisViewRegistry buildDefaultViewRegistry()
        {
        AnalysisViewRegistry registry;
        registry.registerView("symbols", buildSymbolView);
        registry.registerView("dependencies",
            [](const AnalysisGraph& graph)
            {
            AnalysisViewOptions options;
            return buildDependencyView(graph, options);
            });
        return registry;
        }
    }
