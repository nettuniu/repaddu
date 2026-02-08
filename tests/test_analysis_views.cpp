#include "repaddu/analysis_view.h"

#include <cassert>
#include <iostream>

using repaddu::analysis::AnalysisGraph;
using repaddu::analysis::AnalysisViewRegistry;
using repaddu::analysis::AnalysisViewResult;
using repaddu::analysis::EdgeKind;
using repaddu::analysis::SymbolKind;
using repaddu::analysis::SymbolNodeInput;

AnalysisGraph buildGraph()
    {
    AnalysisGraph graph;

    SymbolNodeInput ns;
    ns.kind = SymbolKind::namespace_;
    ns.name = "core";
    ns.qualifiedName = "core";
    ns.containerName = "";
    ns.isPublic = true;
    graph.addSymbol(ns);

    SymbolNodeInput base;
    base.kind = SymbolKind::class_;
    base.name = "Base";
    base.qualifiedName = "core::Base";
    base.containerName = "core";
    base.sourcePath = "src/base/base.cpp";
    base.targetName = "core_lib";
    base.isPublic = true;
    const auto baseId = graph.addSymbol(base);

    SymbolNodeInput derived;
    derived.kind = SymbolKind::class_;
    derived.name = "Derived";
    derived.qualifiedName = "core::Derived";
    derived.containerName = "core";
    derived.sourcePath = "src/derived/derived.cpp";
    derived.targetName = "core_lib";
    derived.isPublic = true;
    const auto derivedId = graph.addSymbol(derived);

    SymbolNodeInput method;
    method.kind = SymbolKind::method_;
    method.name = "run";
    method.qualifiedName = "core::Derived::run";
    method.containerName = "core::Derived";
    method.sourcePath = "src/derived/derived.cpp";
    method.targetName = "core_lib";
    method.isPublic = true;
    const auto methodId = graph.addSymbol(method);

    graph.addEdge(derivedId, baseId, EdgeKind::inherits);
    graph.addEdge(methodId, derivedId, EdgeKind::overrides);

    return graph;
    }

void test_default_views()
    {
    const AnalysisGraph graph = buildGraph();
    const AnalysisViewRegistry registry = repaddu::analysis::buildDefaultViewRegistry();

    assert(registry.hasView("symbols"));
    assert(registry.hasView("dependencies"));

    const AnalysisViewResult symbols = registry.render("symbols", graph);
    assert(symbols.name == "symbols");
    assert(symbols.nodes.size() == 4);
    assert(symbols.edges.size() == 2);

    const AnalysisViewResult deps = registry.render("dependencies", graph);
    assert(deps.name == "dependencies");
    assert(deps.nodes.size() == 3);
    assert(deps.edges.size() == 1);
    }

void test_collapsed_dependency_view()
    {
    const AnalysisGraph graph = buildGraph();
    const AnalysisViewRegistry registry = repaddu::analysis::buildDefaultViewRegistry();

    repaddu::analysis::AnalysisViewOptions options;
    options.collapseMode = "folder";
    const AnalysisViewResult deps = registry.render("dependencies", graph, options);
    assert(deps.nodes.size() == 2);
    }

void test_target_collapsed_dependency_view()
    {
    const AnalysisGraph graph = buildGraph();
    const AnalysisViewRegistry registry = repaddu::analysis::buildDefaultViewRegistry();

    repaddu::analysis::AnalysisViewOptions options;
    options.collapseMode = "target";
    const AnalysisViewResult deps = registry.render("dependencies", graph, options);
    assert(deps.nodes.size() == 1);
    }

int main()
    {
    test_default_views();
    test_collapsed_dependency_view();
    test_target_collapsed_dependency_view();
    std::cout << "Analysis view tests passed." << std::endl;
    return 0;
    }
