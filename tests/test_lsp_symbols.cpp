#include "repaddu/analysis_lsp.h"

#include <cassert>
#include <iostream>

void test_document_symbols()
    {
    const std::string payload =
        "{"
        "\"jsonrpc\":\"2.0\","
        "\"id\":1,"
        "\"result\":["
        "  {\"name\":\"core\",\"kind\":3,\"children\":["
        "      {\"name\":\"Widget\",\"kind\":5,\"children\":["
        "          {\"name\":\"run\",\"kind\":6}"
        "      ]}"
        "  ]}"
        "]"
        "}";

    repaddu::analysis::AnalysisGraph graph;
    const auto result = repaddu::analysis::parseDocumentSymbols(payload, graph);
    assert(result.code == repaddu::core::ExitCode::success);

    const auto* ns = graph.findSymbolByQualifiedName("core");
    assert(ns != nullptr);
    const auto* cls = graph.findSymbolByQualifiedName("core::Widget");
    assert(cls != nullptr);
    const auto* method = graph.findSymbolByQualifiedName("core::Widget::run");
    assert(method != nullptr);
    }

int main()
    {
    test_document_symbols();
    std::cout << "LSP symbol tests passed." << std::endl;
    return 0;
    }
