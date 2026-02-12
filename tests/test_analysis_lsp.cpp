#include "repaddu/analysis_lsp.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace
    {
    std::filesystem::path fixtureRoot()
        {
        const std::filesystem::path root = REPADDU_TEST_ROOT;
        return root / "fixtures" / "analysis_lsp";
        }

    std::string readTextFile(const std::filesystem::path& filePath)
        {
        std::ifstream input(filePath);
        assert(input.good());
        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
        }

    bool hasEdge(const repaddu::analysis::AnalysisGraph& graph, const std::string& from,
        const std::string& to, repaddu::analysis::EdgeKind kind)
        {
        for (const auto& edge : graph.edges())
            {
            const auto& fromSymbol = graph.getSymbol(edge.from);
            const auto& toSymbol = graph.getSymbol(edge.to);
            if (fromSymbol.qualifiedName == from && toSymbol.qualifiedName == to && edge.kind == kind)
                {
                return true;
                }
            }
        return false;
        }
    }

void test_lsp_analysis_integration()
    {
    const std::filesystem::path fixtures = fixtureRoot();

    repaddu::analysis::AnalysisGraph graph;

    const std::string symbolPayload = readTextFile(fixtures / "document_symbols.json");
    const auto symbolsResult = repaddu::analysis::parseDocumentSymbols(symbolPayload, graph);
    assert(symbolsResult.code == repaddu::core::ExitCode::success);

    const auto* ns = graph.findSymbolByQualifiedName("core");
    assert(ns != nullptr);

    const auto* cls = graph.findSymbolByQualifiedName("core::Widget");
    assert(cls != nullptr);

    const auto* method = graph.findSymbolByQualifiedName("core::Widget::run");
    assert(method != nullptr);

    const auto* privateMethod = graph.findSymbolByQualifiedName("core::Widget::debugOnly");
    assert(privateMethod == nullptr);

    repaddu::analysis::SymbolNodeInput origin;
    origin.kind = repaddu::analysis::SymbolKind::class_;
    origin.name = "Origin";
    origin.qualifiedName = "Origin";
    graph.addSymbol(origin);

    repaddu::analysis::SymbolNodeInput iface;
    iface.kind = repaddu::analysis::SymbolKind::class_;
    iface.name = "Interface";
    iface.qualifiedName = "Interface";
    graph.addSymbol(iface);

    repaddu::analysis::LspRelationshipOptions options;
    options.deepEnabled = true;
    options.capabilitySupported = true;

    const std::string supertypesPayload = readTextFile(fixtures / "type_hierarchy_supertypes.json");
    const auto hierarchyResult = repaddu::analysis::parseTypeHierarchySupertypes(
        supertypesPayload, "Origin", graph, options);
    assert(hierarchyResult.code == repaddu::core::ExitCode::success);

    const std::string implPayload = readTextFile(fixtures / "implementations.json");
    const auto implResult = repaddu::analysis::parseImplementationItems(
        implPayload, "Interface", graph, options);
    assert(implResult.code == repaddu::core::ExitCode::success);

    const auto* base = graph.findSymbolByQualifiedName("sample::Base");
    assert(base != nullptr);

    const auto* impl = graph.findSymbolByQualifiedName("sample::Impl");
    assert(impl != nullptr);

    assert(hasEdge(graph, "Origin", "sample::Base", repaddu::analysis::EdgeKind::inherits));
    assert(hasEdge(graph, "Interface", "sample::Impl", repaddu::analysis::EdgeKind::implemented_by));
    }

int main()
    {
    test_lsp_analysis_integration();
    std::cout << "LSP analysis integration tests passed." << std::endl;
    return 0;
    }
