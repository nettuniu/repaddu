#include "repaddu/analysis_lsp.h"

#include <cassert>
#include <iostream>
#include <sstream>

void test_message_roundtrip()
    {
    std::stringstream stream;
    repaddu::analysis::LspClient client(stream, stream);

    const int id = client.sendInitialize("file:///tmp");
    assert(id == 1);

    repaddu::analysis::LspMessage message;
    bool ok = client.readMessage(message);
    assert(ok);
    assert(message.payload.find("\"method\":\"initialize\"") != std::string::npos);
    assert(message.payload.find("\"id\":1") != std::string::npos);
    assert(message.payload.find("\"rootUri\":\"file:///tmp\"") != std::string::npos);

    const int symbolsId = client.requestDocumentSymbols("file:///tmp/main.cpp");
    assert(symbolsId == 2);
    ok = client.readMessage(message);
    assert(ok);
    assert(message.payload.find("\"method\":\"textDocument/documentSymbol\"") != std::string::npos);
    assert(message.payload.find("\"id\":2") != std::string::npos);
    assert(message.payload.find("\"uri\":\"file:///tmp/main.cpp\"") != std::string::npos);
    }

void test_shutdown_notification()
    {
    std::stringstream stream;
    repaddu::analysis::LspClient client(stream, stream);
    client.sendShutdownAndExit();

    repaddu::analysis::LspMessage first;
    bool ok = client.readMessage(first);
    assert(ok);
    assert(first.payload.find("\"method\":\"shutdown\"") != std::string::npos);
    assert(first.payload.find("\"id\":1") != std::string::npos);

    repaddu::analysis::LspMessage second;
    ok = client.readMessage(second);
    assert(ok);
    assert(second.payload.find("\"method\":\"exit\"") != std::string::npos);
    assert(second.payload.find("\"id\":") == std::string::npos);
    }

int main()
    {
    test_message_roundtrip();
    test_shutdown_notification();
    std::cout << "LSP client tests passed." << std::endl;
    return 0;
    }
