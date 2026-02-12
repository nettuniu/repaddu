#ifndef REPADDU_ANALYSIS_LSP_H
#define REPADDU_ANALYSIS_LSP_H

#include "repaddu/analysis_graph.h"
#include "repaddu/core_types.h"
#include "repaddu/json_lite.h"

#include <istream>
#include <ostream>
#include <string>

namespace repaddu::analysis
    {
    struct LspRelationshipOptions
        {
        bool deepEnabled = false;
        bool capabilitySupported = false;
        };

    struct LspMessage
        {
        std::string payload;
        };

    class LspMessageIO
        {
        public:
            static void writeMessage(std::ostream& out, const std::string& payload);
            static bool readMessage(std::istream& in, LspMessage& message);
        };

    class LspClient
        {
        public:
            LspClient(std::istream& in, std::ostream& out);

            int sendRequest(const std::string& method, const std::string& paramsJson);
            void sendNotification(const std::string& method, const std::string& paramsJson);
            bool readMessage(LspMessage& message);
            int sendInitialize(const std::string& rootUri);
            int requestDocumentSymbols(const std::string& documentUri);
            void sendShutdownAndExit();

        private:
            std::istream& in_;
            std::ostream& out_;
            int nextId_ = 1;
        };

    core::RunResult parseDocumentSymbols(const std::string& jsonPayload, AnalysisGraph& graph);
    core::RunResult parseTypeHierarchySupertypes(const std::string& jsonPayload,
        const std::string& originQualifiedName, AnalysisGraph& graph,
        const LspRelationshipOptions& options = {});
    core::RunResult parseImplementationItems(const std::string& jsonPayload,
        const std::string& originQualifiedName, AnalysisGraph& graph,
        const LspRelationshipOptions& options = {});
    }

#endif // REPADDU_ANALYSIS_LSP_H
