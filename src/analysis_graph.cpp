#include "repaddu/analysis_graph.h"

#include <stdexcept>

namespace repaddu::analysis
    {
    SymbolId AnalysisGraph::addSymbol(const SymbolNodeInput& input)
        {
        auto it = byQualifiedName_.find(input.qualifiedName);
        if (it != byQualifiedName_.end())
            {
            return it->second;
            }
        return addSymbolInternal(input);
        }

    SymbolId AnalysisGraph::addSymbolInternal(const SymbolNodeInput& input)
        {
        SymbolId id = symbols_.size();
        SymbolNode node;
        node.id = id;
        node.kind = input.kind;
        node.name = input.name;
        node.qualifiedName = input.qualifiedName;
        node.containerName = input.containerName;
        node.sourcePath = input.sourcePath;
        node.targetName = input.targetName;
        node.isPublic = input.isPublic;
        symbols_.push_back(node);
        byQualifiedName_.emplace(node.qualifiedName, id);
        return id;
        }

    bool AnalysisGraph::addEdge(SymbolId from, SymbolId to, EdgeKind kind)
        {
        if (from >= symbols_.size() || to >= symbols_.size())
            {
            return false;
            }
        const auto key = std::make_tuple(from, to, kind);
        if (edgeSet_.count(key) != 0)
            {
            return false;
            }
        edgeSet_.insert(key);
        edges_.push_back(SymbolEdge{ from, to, kind });
        return true;
        }

    const SymbolNode* AnalysisGraph::findSymbolByQualifiedName(const std::string& qualifiedName) const
        {
        auto it = byQualifiedName_.find(qualifiedName);
        if (it == byQualifiedName_.end())
            {
            return nullptr;
            }
        return &symbols_.at(it->second);
        }

    const SymbolNode& AnalysisGraph::getSymbol(SymbolId id) const
        {
        if (id >= symbols_.size())
            {
            throw std::out_of_range("SymbolId out of range");
            }
        return symbols_[id];
        }

    const std::vector<SymbolNode>& AnalysisGraph::symbols() const
        {
        return symbols_;
        }

    const std::vector<SymbolEdge>& AnalysisGraph::edges() const
        {
        return edges_;
        }

    std::vector<SymbolId> AnalysisGraph::publicSymbols() const
        {
        std::vector<SymbolId> result;
        result.reserve(symbols_.size());
        for (const auto& symbol : symbols_)
            {
            if (symbol.isPublic)
                {
                result.push_back(symbol.id);
                }
            }
        return result;
        }
    }
