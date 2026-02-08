#include "repaddu/analysis_lsp.h"

#include <cassert>
#include <iostream>

void test_hierarchy_edges()
    {
    repaddu::analysis::AnalysisGraph graph;
    repaddu::analysis::SymbolNodeInput origin;
    origin.kind = repaddu::analysis::SymbolKind::class_;
    origin.name = "Origin";
    origin.qualifiedName = "Origin";
    graph.addSymbol(origin);

    const std::string supertypes =
        "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":2,"
        "\"result\":["
        "  {\"name\":\"Base\",\"kind\":5}"
        "]"
        "}";

    const auto result = repaddu::analysis::parseTypeHierarchySupertypes(supertypes, "Origin", graph);
    assert(result.code == repaddu::core::ExitCode::success);

    const auto* base = graph.findSymbolByQualifiedName("Base");
    assert(base != nullptr);

    bool sawInherits = false;
    for (const auto& edge : graph.edges())
        {
        const auto& from = graph.getSymbol(edge.from);
        const auto& to = graph.getSymbol(edge.to);
        if (from.qualifiedName == "Origin" && to.qualifiedName == "Base"
            && edge.kind == repaddu::analysis::EdgeKind::inherits)
            {
            sawInherits = true;
            }
        }
    assert(sawInherits);
    }

void test_implementation_edges()
    {
    repaddu::analysis::AnalysisGraph graph;
    repaddu::analysis::SymbolNodeInput origin;
    origin.kind = repaddu::analysis::SymbolKind::class_;
    origin.name = "Interface";
    origin.qualifiedName = "Interface";
    graph.addSymbol(origin);

    const std::string impls =
        "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":3,"
        "\"result\":["
        "  {\"name\":\"Impl\",\"kind\":5}"
        "]"
        "}";

    const auto result = repaddu::analysis::parseImplementationItems(impls, "Interface", graph);
    assert(result.code == repaddu::core::ExitCode::success);

    bool sawImplemented = false;
    for (const auto& edge : graph.edges())
        {
        const auto& from = graph.getSymbol(edge.from);
        const auto& to = graph.getSymbol(edge.to);
        if (from.qualifiedName == "Interface" && to.qualifiedName == "Impl"
            && edge.kind == repaddu::analysis::EdgeKind::implemented_by)
            {
            sawImplemented = true;
            }
        }
    assert(sawImplemented);
    }

int main()
    {
    test_hierarchy_edges();
    test_implementation_edges();
    std::cout << "LSP relationship tests passed." << std::endl;
    return 0;
    }
