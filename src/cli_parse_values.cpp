#include "cli_parse_values.h"

#include <sstream>

namespace repaddu::cli::detail
    {
    std::vector<std::string> splitCsv(const std::string& value)
        {
        std::vector<std::string> result;
        std::string current;
        std::istringstream stream(value);
        while (std::getline(stream, current, ','))
            {
            if (!current.empty())
                {
                result.push_back(current);
                }
            }
        return result;
        }

    bool parseInt(const std::string& value, int& outValue)
        {
        try
            {
            std::size_t pos = 0;
            const int parsed = std::stoi(value, &pos, 10);
            if (pos != value.size())
                {
                return false;
                }
            outValue = parsed;
            return true;
            }
        catch (...)
            {
            return false;
            }
        }

    bool parseUInt64(const std::string& value, std::uintmax_t& outValue)
        {
        try
            {
            std::size_t pos = 0;
            const unsigned long long parsed = std::stoull(value, &pos, 10);
            if (pos != value.size())
                {
                return false;
                }
            outValue = static_cast<std::uintmax_t>(parsed);
            return true;
            }
        catch (...)
            {
            return false;
            }
        }
    }
