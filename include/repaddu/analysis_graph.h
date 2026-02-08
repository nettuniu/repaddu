#ifndef REPADDU_ANALYSIS_GRAPH_H
#define REPADDU_ANALYSIS_GRAPH_H

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace repaddu::analysis
    {
    using SymbolId = std::size_t;

    enum class SymbolKind
        {
        module,
        namespace_,
        class_,
        method_
        };

    enum class EdgeKind
        {
        inherits,
        overrides,
        implemented_by
        };

    struct SymbolNode
        {
        SymbolId id = 0;
        SymbolKind kind = SymbolKind::class_;
        std::string name;
        std::string qualifiedName;
        std::string containerName; // namespace or module name
        std::string sourcePath;
        std::string targetName;
        bool isPublic = true;
        };

    struct SymbolNodeInput
        {
        SymbolKind kind = SymbolKind::class_;
        std::string name;
        std::string qualifiedName;
        std::string containerName;
        std::string sourcePath;
        std::string targetName;
        bool isPublic = true;
        };

    struct SymbolEdge
        {
        SymbolId from = 0;
        SymbolId to = 0;
        EdgeKind kind = EdgeKind::inherits;
        };

    class AnalysisGraph
        {
        public:
            SymbolId addSymbol(const SymbolNodeInput& input);
            bool addEdge(SymbolId from, SymbolId to, EdgeKind kind);

            const SymbolNode* findSymbolByQualifiedName(const std::string& qualifiedName) const;
            const SymbolNode& getSymbol(SymbolId id) const;

            const std::vector<SymbolNode>& symbols() const;
            const std::vector<SymbolEdge>& edges() const;

            std::vector<SymbolId> publicSymbols() const;

        private:
            SymbolId addSymbolInternal(const SymbolNodeInput& input);

            std::vector<SymbolNode> symbols_;
            std::vector<SymbolEdge> edges_;
            std::map<std::string, SymbolId> byQualifiedName_;
            std::set<std::tuple<SymbolId, SymbolId, EdgeKind>> edgeSet_;
        };
    }

#endif // REPADDU_ANALYSIS_GRAPH_H
