#ifndef REPADDU_CLI_PARSE_VALUES_H
#define REPADDU_CLI_PARSE_VALUES_H

#include <cstdint>
#include <string>
#include <vector>

namespace repaddu::cli::detail
    {
    std::vector<std::string> splitCsv(const std::string& value);
    bool parseInt(const std::string& value, int& outValue);
    bool parseUInt64(const std::string& value, std::uintmax_t& outValue);
    }

#endif // REPADDU_CLI_PARSE_VALUES_H
