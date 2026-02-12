#include "repaddu/analysis_lsp.h"

#include <cctype>
#include <sstream>

namespace repaddu::analysis
    {
    namespace
        {
        std::string trim(const std::string& value)
            {
            std::size_t start = 0;
            while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0)
                {
                ++start;
                }
            std::size_t end = value.size();
            while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0)
                {
                --end;
                }
            return value.substr(start, end - start);
            }

        bool isPublicAccess(const json::JsonObject& obj)
            {
            auto it = obj.find("access");
            if (it == obj.end() || !it->second.isString())
                {
                return true;
                }
            const std::string access = it->second.getString();
            return access == "public";
            }

        SymbolKind toSymbolKind(int kind)
            {
            switch (kind)
                {
                case 2:  // Module
                case 3:  // Namespace
                case 4:  // Package
                    return SymbolKind::namespace_;
                case 5:  // Class
                case 11: // Interface
                case 23: // Struct
                    return SymbolKind::class_;
                case 6:  // Method
                case 9:  // Constructor
                case 12: // Function
                    return SymbolKind::method_;
                default:
                    return SymbolKind::method_;
                }
            }

        void addSymbolFromJson(const json::JsonObject& obj, AnalysisGraph& graph,
            const std::string& container)
            {
            if (!obj.count("name") || !obj.at("name").isString())
                {
                return;
                }
            if (!obj.count("kind") || !obj.at("kind").isNumber())
                {
                return;
                }
            if (!isPublicAccess(obj))
                {
                return;
                }

            const std::string name = obj.at("name").getString();
            const int kindValue = static_cast<int>(obj.at("kind").getNumber());
            const SymbolKind kind = toSymbolKind(kindValue);

            if (kind != SymbolKind::class_ && kind != SymbolKind::method_ && kind != SymbolKind::namespace_)
                {
                return;
                }

            SymbolNodeInput input;
            input.kind = kind;
            input.name = name;
            input.containerName = container;
            if (container.empty())
                {
                input.qualifiedName = name;
                }
            else
                {
                input.qualifiedName = container + "::" + name;
                }
            input.isPublic = true;
            graph.addSymbol(input);

            auto it = obj.find("children");
            if (it != obj.end() && it->second.isArray())
                {
                const auto& children = it->second.getArray();
                for (const auto& child : children)
                    {
                    if (!child.isObject())
                        {
                        continue;
                        }
                    addSymbolFromJson(child.getObject(), graph, input.qualifiedName);
                    }
                }
            }

        void addSymbolInformationArray(const json::JsonArray& items, AnalysisGraph& graph)
            {
            for (const auto& entry : items)
                {
                if (!entry.isObject())
                    {
                    continue;
                    }
                const auto& obj = entry.getObject();
                if (!obj.count("name") || !obj.at("name").isString())
                    {
                    continue;
                    }
                if (!obj.count("kind") || !obj.at("kind").isNumber())
                    {
                    continue;
                    }
                if (!isPublicAccess(obj))
                    {
                    continue;
                    }

                const std::string name = obj.at("name").getString();
                const int kindValue = static_cast<int>(obj.at("kind").getNumber());
                const SymbolKind kind = toSymbolKind(kindValue);
                if (kind != SymbolKind::class_ && kind != SymbolKind::method_ && kind != SymbolKind::namespace_)
                    {
                    continue;
                    }

                std::string container;
                auto containerIt = obj.find("containerName");
                if (containerIt != obj.end() && containerIt->second.isString())
                    {
                    container = containerIt->second.getString();
                    }

                SymbolNodeInput input;
                input.kind = kind;
                input.name = name;
                input.containerName = container;
                if (container.empty())
                    {
                    input.qualifiedName = name;
                    }
                else
                    {
                    input.qualifiedName = container + "::" + name;
                    }
                input.isPublic = true;
                graph.addSymbol(input);
                }
            }

        core::RunResult parseJson(const std::string& jsonPayload, json::JsonValue& root)
            {
            return json::parse(jsonPayload, root);
            }

        void addHierarchyEdges(const json::JsonArray& items, const std::string& originQualifiedName,
            AnalysisGraph& graph, EdgeKind kind)
            {
            for (const auto& entry : items)
                {
                if (!entry.isObject())
                    {
                    continue;
                    }
                const auto& obj = entry.getObject();
                if (!obj.count("name") || !obj.at("name").isString())
                    {
                    continue;
                    }
                if (!obj.count("kind") || !obj.at("kind").isNumber())
                    {
                    continue;
                    }

                const std::string name = obj.at("name").getString();
                std::string container;
                auto it = obj.find("detail");
                if (it != obj.end() && it->second.isString())
                    {
                    container = trim(it->second.getString());
                    }

                SymbolNodeInput input;
                input.name = name;
                input.kind = SymbolKind::class_;
                input.containerName = container;
                if (container.empty())
                    {
                    input.qualifiedName = name;
                    }
                else
                    {
                    input.qualifiedName = container + "::" + name;
                    }
                const SymbolId targetId = graph.addSymbol(input);
                const SymbolNode* origin = graph.findSymbolByQualifiedName(originQualifiedName);
                if (!origin)
                    {
                    continue;
                    }
                graph.addEdge(origin->id, targetId, kind);
                }
            }
        }

    void LspMessageIO::writeMessage(std::ostream& out, const std::string& payload)
        {
        out << "Content-Length: " << payload.size() << "\r\n\r\n";
        out << payload;
        out.flush();
        }

    bool LspMessageIO::readMessage(std::istream& in, LspMessage& message)
        {
        std::string line;
        std::size_t contentLength = 0;
        while (std::getline(in, line))
            {
            if (!line.empty() && line.back() == '\r')
                {
                line.pop_back();
                }
            if (line.empty())
                {
                break;
                }
            const std::string prefix = "Content-Length:";
            if (line.rfind(prefix, 0) == 0)
                {
                const std::string value = trim(line.substr(prefix.size()));
                contentLength = static_cast<std::size_t>(std::stoul(value));
                }
            }

        if (contentLength == 0)
            {
            return false;
            }

        std::string payload(contentLength, '\0');
        in.read(payload.data(), static_cast<std::streamsize>(contentLength));
        if (!in)
            {
            return false;
            }
        message.payload = payload;
        return true;
        }

    LspClient::LspClient(std::istream& in, std::ostream& out)
        : in_(in),
          out_(out)
        {
        }

    int LspClient::sendRequest(const std::string& method, const std::string& paramsJson)
        {
        const int id = nextId_++;
        std::ostringstream payload;
        payload << "{\"jsonrpc\":\"2.0\",\"id\":" << id << ",\"method\":\"" << method << "\"";
        if (!paramsJson.empty())
            {
            payload << ",\"params\":" << paramsJson;
            }
        payload << "}";
        LspMessageIO::writeMessage(out_, payload.str());
        return id;
        }

    void LspClient::sendNotification(const std::string& method, const std::string& paramsJson)
        {
        std::ostringstream payload;
        payload << "{\"jsonrpc\":\"2.0\",\"method\":\"" << method << "\"";
        if (!paramsJson.empty())
            {
            payload << ",\"params\":" << paramsJson;
            }
        payload << "}";
        LspMessageIO::writeMessage(out_, payload.str());
        }

    bool LspClient::readMessage(LspMessage& message)
        {
        return LspMessageIO::readMessage(in_, message);
        }

    core::RunResult parseDocumentSymbols(const std::string& jsonPayload, AnalysisGraph& graph)
        {
        json::JsonValue root;
        core::RunResult parseResult = parseJson(jsonPayload, root);
        if (parseResult.code != core::ExitCode::success)
            {
            return parseResult;
            }
        if (!root.isObject())
            {
            return { core::ExitCode::invalid_usage, "LSP response must be an object." };
            }

        const auto& obj = root.getObject();
        auto it = obj.find("result");
        if (it == obj.end())
            {
            return { core::ExitCode::invalid_usage, "LSP response missing result." };
            }

        if (it->second.isArray())
            {
            const auto& array = it->second.getArray();
            if (!array.empty() && array.front().isObject() && array.front().getObject().count("children") != 0)
                {
                for (const auto& entry : array)
                    {
                    if (!entry.isObject())
                        {
                        continue;
                        }
                    addSymbolFromJson(entry.getObject(), graph, "");
                    }
                return { core::ExitCode::success, "" };
                }

            addSymbolInformationArray(array, graph);
            return { core::ExitCode::success, "" };
            }

        return { core::ExitCode::invalid_usage, "Unsupported LSP documentSymbol result format." };
        }

    core::RunResult parseTypeHierarchySupertypes(const std::string& jsonPayload,
        const std::string& originQualifiedName, AnalysisGraph& graph,
        const LspRelationshipOptions& options)
        {
        if (!options.deepEnabled || !options.capabilitySupported)
            {
            return { core::ExitCode::success, "" };
            }

        json::JsonValue root;
        core::RunResult parseResult = parseJson(jsonPayload, root);
        if (parseResult.code != core::ExitCode::success)
            {
            return parseResult;
            }
        if (!root.isObject())
            {
            return { core::ExitCode::invalid_usage, "LSP response must be an object." };
            }

        const auto& obj = root.getObject();
        auto it = obj.find("result");
        if (it == obj.end() || !it->second.isArray())
            {
            return { core::ExitCode::invalid_usage, "LSP response missing result array." };
            }

        addHierarchyEdges(it->second.getArray(), originQualifiedName, graph, EdgeKind::inherits);
        return { core::ExitCode::success, "" };
        }

    core::RunResult parseImplementationItems(const std::string& jsonPayload,
        const std::string& originQualifiedName, AnalysisGraph& graph,
        const LspRelationshipOptions& options)
        {
        if (!options.deepEnabled || !options.capabilitySupported)
            {
            return { core::ExitCode::success, "" };
            }

        json::JsonValue root;
        core::RunResult parseResult = parseJson(jsonPayload, root);
        if (parseResult.code != core::ExitCode::success)
            {
            return parseResult;
            }
        if (!root.isObject())
            {
            return { core::ExitCode::invalid_usage, "LSP response must be an object." };
            }

        const auto& obj = root.getObject();
        auto it = obj.find("result");
        if (it == obj.end() || !it->second.isArray())
            {
            return { core::ExitCode::invalid_usage, "LSP response missing result array." };
            }

        addHierarchyEdges(it->second.getArray(), originQualifiedName, graph, EdgeKind::implemented_by);
        return { core::ExitCode::success, "" };
        }
    }
