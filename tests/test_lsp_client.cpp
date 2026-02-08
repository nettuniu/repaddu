#include "repaddu/analysis_lsp.h"

#include <cassert>
#include <iostream>
#include <sstream>

void test_message_roundtrip()
    {
    std::stringstream stream;
    repaddu::analysis::LspClient client(stream, stream);

    const int id = client.sendRequest("initialize", "{\"rootUri\":\"file:///tmp\"}");
    assert(id == 1);

    repaddu::analysis::LspMessage message;
    bool ok = client.readMessage(message);
    assert(ok);
    assert(message.payload.find("\"method\":\"initialize\"") != std::string::npos);
    assert(message.payload.find("\"id\":1") != std::string::npos);
    }

int main()
    {
    test_message_roundtrip();
    std::cout << "LSP client tests passed." << std::endl;
    return 0;
    }
