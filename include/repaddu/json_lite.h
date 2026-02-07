#ifndef REPADDU_JSON_LITE_H
#define REPADDU_JSON_LITE_H

#include "repaddu/core_types.h"
#include <string>
#include <vector>
#include <map>
#include <variant>

namespace repaddu::json
    {
    struct JsonValue;
    using JsonObject = std::map<std::string, JsonValue>;
    using JsonArray = std::vector<JsonValue>;
    
    struct JsonValue
        {
        std::variant<std::monostate, bool, double, std::string, JsonObject, JsonArray> data;

        bool isBool() const { return std::holds_alternative<bool>(data); }
        bool getBool() const { return std::get<bool>(data); }

        bool isNumber() const { return std::holds_alternative<double>(data); }
        double getNumber() const { return std::get<double>(data); }

        bool isString() const { return std::holds_alternative<std::string>(data); }
        std::string getString() const { return std::get<std::string>(data); }

        bool isObject() const { return std::holds_alternative<JsonObject>(data); }
        const JsonObject& getObject() const { return std::get<JsonObject>(data); }

        bool isArray() const { return std::holds_alternative<JsonArray>(data); }
        const JsonArray& getArray() const { return std::get<JsonArray>(data); }
        };

    core::RunResult parse(const std::string& input, JsonValue& outValue);
    }

#endif // REPADDU_JSON_LITE_H
