#include "repaddu/grouping_component_map.h"

#include "repaddu/core_types.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace repaddu::grouping
    {
    namespace
        {
        struct Parser
            {
            const std::string& input;
            std::size_t pos = 0;

            void skipWhitespace()
                {
                while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos])) != 0)
                    {
                    ++pos;
                    }
                }

            bool consume(char expected)
                {
                skipWhitespace();
                if (pos < input.size() && input[pos] == expected)
                    {
                    ++pos;
                    return true;
                    }
                return false;
                }

            bool parseString(std::string& outValue)
                {
                skipWhitespace();
                if (pos >= input.size() || input[pos] != '"')
                    {
                    return false;
                    }
                ++pos;
                std::string result;
                while (pos < input.size())
                    {
                    char ch = input[pos++];
                    if (ch == '"')
                        {
                        outValue = result;
                        return true;
                        }
                    if (ch == '\\')
                        {
                        if (pos >= input.size())
                            {
                            return false;
                            }
                        char escaped = input[pos++];
                        switch (escaped)
                            {
                            case '"': result.push_back('"'); break;
                            case '\\': result.push_back('\\'); break;
                            case '/': result.push_back('/'); break;
                            case 'b': result.push_back('\b'); break;
                            case 'f': result.push_back('\f'); break;
                            case 'n': result.push_back('\n'); break;
                            case 'r': result.push_back('\r'); break;
                            case 't': result.push_back('\t'); break;
                            default: return false;
                            }
                        }
                    else
                        {
                        result.push_back(ch);
                        }
                    }
                return false;
                }

            bool parseStringArray(std::vector<std::string>& outValues)
                {
                if (!consume('['))
                    {
                    return false;
                    }
                skipWhitespace();
                if (consume(']'))
                    {
                    return true;
                    }
                while (true)
                    {
                    std::string value;
                    if (!parseString(value))
                        {
                        return false;
                        }
                    outValues.push_back(value);
                    skipWhitespace();
                    if (consume(']'))
                        {
                        return true;
                        }
                    if (!consume(','))
                        {
                        return false;
                        }
                    }
                }
            };

        std::string normalizePathPrefix(std::string value)
            {
            for (char& ch : value)
                {
                if (ch == '\\')
                    {
                    ch = '/';
                    }
                }
            return value;
            }
        }

    core::RunResult loadComponentMap(const std::filesystem::path& path, ComponentMap& outMap)
        {
        outMap = ComponentMap{};

        std::ifstream stream(path);
        if (!stream)
            {
            return { core::ExitCode::io_failure, "Failed to open component map." };
            }

        std::ostringstream buffer;
        buffer << stream.rdbuf();
        const std::string content = buffer.str();

        Parser parser{ content };
        if (!parser.consume('{'))
            {
            return { core::ExitCode::invalid_usage, "Component map must be a JSON object." };
            }

        parser.skipWhitespace();
        if (parser.consume('}'))
            {
            return { core::ExitCode::invalid_usage, "Component map is empty." };
            }

        while (true)
            {
            std::string key;
            if (!parser.parseString(key))
                {
                return { core::ExitCode::invalid_usage, "Invalid JSON key in component map." };
                }
            if (!parser.consume(':'))
                {
                return { core::ExitCode::invalid_usage, "Expected ':' after component key." };
                }
            std::vector<std::string> prefixes;
            if (!parser.parseStringArray(prefixes))
                {
                return { core::ExitCode::invalid_usage, "Component map values must be arrays of strings." };
                }
            for (std::string& prefix : prefixes)
                {
                prefix = normalizePathPrefix(prefix);
                if (!prefix.empty() && prefix.front() == '/')
                    {
                    prefix.erase(prefix.begin());
                    }
                }
            outMap.componentToPrefixes.emplace(std::move(key), std::move(prefixes));

            parser.skipWhitespace();
            if (parser.consume('}'))
                {
                break;
                }
            if (!parser.consume(','))
                {
                return { core::ExitCode::invalid_usage, "Expected ',' between component entries." };
                }
            }

        return { core::ExitCode::success, "" };
        }

    std::string resolveComponent(const ComponentMap& map, const std::filesystem::path& relativePath)
        {
        const std::string normalized = relativePath.generic_string();
        std::string bestComponent = "unmapped";
        std::size_t bestLength = 0;

        for (const auto& entry : map.componentToPrefixes)
            {
            for (const std::string& prefixRaw : entry.second)
                {
                const std::string prefix = normalizePathPrefix(prefixRaw);
                if (prefix.empty())
                    {
                    continue;
                    }
                if (normalized.rfind(prefix, 0) == 0)
                    {
                    const std::size_t length = prefix.size();
                    if (length > bestLength)
                        {
                        bestLength = length;
                        bestComponent = entry.first;
                        }
                    }
                }
            }

        return bestComponent;
        }
    }
