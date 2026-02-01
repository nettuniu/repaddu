#include "repaddu/core_types.h"

#include <algorithm>
#include <cctype>

namespace repaddu::core
    {
    std::string toLowerCopy(std::string_view value)
        {
        std::string result;
        result.reserve(value.size());
        for (unsigned char ch : value)
            {
            result.push_back(static_cast<char>(std::tolower(ch)));
            }
        return result;
        }

    std::string sanitizeName(std::string_view value)
        {
        std::string result;
        result.reserve(value.size());
        for (unsigned char ch : value)
            {
            if (std::isalnum(ch) != 0)
                {
                result.push_back(static_cast<char>(std::tolower(ch)));
                }
            else
                {
                result.push_back('_');
                }
            }
        while (!result.empty() && result.front() == '_')
            {
            result.erase(result.begin());
            }
        while (!result.empty() && result.back() == '_')
            {
            result.pop_back();
            }
        if (result.empty())
            {
            result = "group";
            }
        return result;
        }

    std::string fileClassLabel(FileClass value)
        {
        switch (value)
            {
            case FileClass::header:
                return "header";
            case FileClass::source:
                return "source";
            case FileClass::other:
                return "other";
            }
        return "other";
        }

    FileClass classifyExtension(std::string_view extensionLower)
        {
        if (extensionLower == ".h" || extensionLower == ".hpp" || extensionLower == ".hh" || extensionLower == ".hxx")
            {
            return FileClass::header;
            }
        if (extensionLower == ".c" || extensionLower == ".cc" || extensionLower == ".cpp" || extensionLower == ".cxx")
            {
            return FileClass::source;
            }
        return FileClass::other;
        }
    }
