#include "repaddu/json_lite.h"
#include "repaddu/core_types.h"
#include <cctype>
#include <iostream>

namespace repaddu::json
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

            bool peek(char expected)
                {
                skipWhitespace();
                return pos < input.size() && input[pos] == expected;
                }

            bool parseString(std::string& out)
                {
                if (!consume('"')) return false;
                std::string result;
                while (pos < input.size())
                    {
                    char ch = input[pos++];
                    if (ch == '"') { out = result; return true; }
                    if (ch == '\\')
                        {
                        if (pos >= input.size()) return false;
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
                    else result.push_back(ch);
                    }
                return false;
                }

            bool parseNumber(double& out)
                {
                skipWhitespace();
                std::size_t start = pos;
                if (pos < input.size() && input[pos] == '-') pos++;
                while (pos < input.size() && (std::isdigit(input[pos]) || input[pos] == '.')) pos++;
                if (start == pos) return false;
                try
                    {
                    out = std::stod(input.substr(start, pos - start));
                    return true;
                    }
                catch (...) { return false; }
                }

            bool parseLiteral(const std::string& lit, JsonValue& out, JsonValue val)
                {
                skipWhitespace();
                if (input.substr(pos, lit.size()) == lit)
                    {
                    pos += lit.size();
                    out = val;
                    return true;
                    }
                return false;
                }

            bool parseValue(JsonValue& out)
                {
                skipWhitespace();
                if (pos >= input.size()) return false;

                if (peek('{')) return parseObject(out);
                if (peek('[')) return parseArray(out);
                if (peek('"')) { std::string s; if (parseString(s)) { out.data = s; return true; } return false; }
                if (peek('t')) return parseLiteral("true", out, JsonValue{true});
                if (peek('f')) return parseLiteral("false", out, JsonValue{false});
                if (peek('n')) return parseLiteral("null", out, JsonValue{std::monostate{}});
                
                double d;
                if (parseNumber(d)) { out.data = d; return true; }
                return false;
                }

            bool parseObject(JsonValue& out)
                {
                if (!consume('{')) return false;
                JsonObject obj;
                if (consume('}')) { out.data = obj; return true; }
                while (true)
                    {
                    std::string key;
                    if (!parseString(key)) return false;
                    if (!consume(':')) return false;
                    JsonValue val;
                    if (!parseValue(val)) return false;
                    obj[key] = val;
                    if (consume('}')) { out.data = obj; return true; }
                    if (!consume(',')) return false;
                    }
                }

            bool parseArray(JsonValue& out)
                {
                if (!consume('[')) return false;
                JsonArray arr;
                if (consume(']')) { out.data = arr; return true; }
                while (true)
                    {
                    JsonValue val;
                    if (!parseValue(val)) return false;
                    arr.push_back(val);
                    if (consume(']')) { out.data = arr; return true; }
                    if (!consume(',')) return false;
                    }
                }
            };
        }

    core::RunResult parse(const std::string& input, JsonValue& outValue)
        {
        Parser p{input};
        if (p.parseValue(outValue)) return {core::ExitCode::success, ""};
        return {core::ExitCode::invalid_usage, "JSON parse error"};
        }
    }
