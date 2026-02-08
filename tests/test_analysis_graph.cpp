#include "repaddu/analysis_graph.h"
#include <cassert>
#include <iostream>

using repaddu::analysis::AnalysisGraph;
using repaddu::analysis::EdgeKind;
using repaddu::analysis::SymbolKind;
using repaddu::analysis::SymbolNodeInput;

void test_graph_basics()
    {
    AnalysisGraph graph;

    SymbolNodeInput ns;
    ns.kind = SymbolKind::namespace_;
    ns.name = "example";
    ns.qualifiedName = "example";
    ns.containerName = "";
    ns.isPublic = true;
    const auto nsId = graph.addSymbol(ns);

    SymbolNodeInput cls;
    cls.kind = SymbolKind::class_;
    cls.name = "Widget";
    cls.qualifiedName = "example::Widget";
    cls.containerName = "example";
    cls.isPublic = true;
    const auto clsId = graph.addSymbol(cls);

    SymbolNodeInput method;
    method.kind = SymbolKind::method_;
    method.name = "run";
    method.qualifiedName = "example::Widget::run";
    method.containerName = "example::Widget";
    method.isPublic = true;
    const auto methodId = graph.addSymbol(method);

    assert(nsId == 0);
    assert(clsId == 1);
    assert(methodId == 2);

    const auto* found = graph.findSymbolByQualifiedName("example::Widget");
    assert(found != nullptr);
    assert(found->id == clsId);

    const auto* missing = graph.findSymbolByQualifiedName("missing");
    assert(missing == nullptr);

    assert(graph.addEdge(clsId, methodId, EdgeKind::overrides) == true);
    assert(graph.addEdge(clsId, methodId, EdgeKind::overrides) == false);
    assert(graph.addEdge(999, clsId, EdgeKind::inherits) == false);

    const auto publicIds = graph.publicSymbols();
    assert(publicIds.size() == 3);
    assert(publicIds[0] == nsId);
    assert(publicIds[1] == clsId);
    assert(publicIds[2] == methodId);
    }

int main()
    {
    test_graph_basics();
    std::cout << "Analysis graph tests passed." << std::endl;
    return 0;
    }
