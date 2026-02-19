#include "repaddu/analysis_tags.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace repaddu::analysis
    {
    namespace
        {
        bool isWordChar(char value)
            {
            return std::isalnum(static_cast<unsigned char>(value)) != 0 || value == '_';
            }

        std::string toUpperCopy(const std::string& value)
            {
            std::string upper = value;
            std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char ch)
                {
                return static_cast<char>(std::toupper(ch));
                });
            return upper;
            }

        std::string trimCopy(const std::string& value)
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

        bool findTagStart(const std::string& line,
            const std::string& lineUpper,
            const std::string& tag,
            const std::string& tagUpper,
            std::size_t& outTagStart)
            {
            std::size_t searchStart = 0;
            while (searchStart < lineUpper.size())
                {
                const std::size_t pos = lineUpper.find(tagUpper, searchStart);
                if (pos == std::string::npos)
                    {
                    return false;
                    }

                bool prefixIsAt = false;
                if (pos > 0 && line[pos - 1] == '@')
                    {
                    prefixIsAt = true;
                    if (pos > 1 && isWordChar(line[pos - 2]))
                        {
                        searchStart = pos + 1;
                        continue;
                        }
                    }
                else if (pos > 0 && isWordChar(line[pos - 1]))
                    {
                    searchStart = pos + 1;
                    continue;
                    }

                const std::size_t suffixPos = pos + tagUpper.size();
                if (suffixPos < line.size() && isWordChar(line[suffixPos]))
                    {
                    searchStart = pos + 1;
                    continue;
                    }

                if (!prefixIsAt && line.compare(pos, tag.size(), tag) != 0)
                    {
                    searchStart = pos + 1;
                    continue;
                    }

                outTagStart = prefixIsAt ? (pos - 1) : pos;
                return true;
                }

            return false;
            }

        void appendMatchesForLine(const std::string& line,
            const std::vector<std::string>& tags,
            int lineNum,
            const std::string& filePath,
            std::vector<TagMatch>& results)
            {
            const std::string lineUpper = toUpperCopy(line);
            for (const auto& tag : tags)
                {
                if (tag.empty())
                    {
                    continue;
                    }

                const std::string tagUpper = toUpperCopy(tag);
                std::size_t tagStart = 0;
                if (!findTagStart(line, lineUpper, tag, tagUpper, tagStart))
                    {
                    continue;
                    }

                TagMatch match;
                match.tag = tag;
                match.lineNumber = lineNum;
                match.filePath = filePath;

                std::size_t contentPos = tagStart;
                if (line[contentPos] == '@')
                    {
                    ++contentPos;
                    }
                contentPos += tag.length();
                while (contentPos < line.length() && (line[contentPos] == ':' || std::isspace(static_cast<unsigned char>(line[contentPos])) != 0))
                    {
                    ++contentPos;
                    }

                match.content = line.substr(contentPos);
                const std::size_t last = match.content.find_last_not_of(" \t\n\r");
                if (last != std::string::npos)
                    {
                    match.content = match.content.substr(0, last + 1);
                    }

                results.push_back(std::move(match));
                }
            }
        }

    TagExtractor::TagExtractor()
        {
        tags_ = {"TODO", "FIXME", "BUG", "HACK"};
        }

    void TagExtractor::addTagPattern(const std::string& tag)
        {
        if (std::find(tags_.begin(), tags_.end(), tag) == tags_.end())
            {
            tags_.push_back(tag);
            }
        }

    bool TagExtractor::loadTagPatternsFromFile(const std::filesystem::path& path, bool replaceExisting)
        {
        std::ifstream input(path);
        if (!input)
            {
            return false;
            }

        std::vector<std::string> loadedTags;
        std::string line;
        while (std::getline(input, line))
            {
            const std::string trimmed = trimCopy(line);
            if (trimmed.empty())
                {
                continue;
                }
            if (!trimmed.empty() && trimmed.front() == '#')
                {
                continue;
                }
            if (std::find(loadedTags.begin(), loadedTags.end(), trimmed) == loadedTags.end())
                {
                loadedTags.push_back(trimmed);
                }
            }

        if (replaceExisting)
            {
            tags_.clear();
            }
        for (const auto& tag : loadedTags)
            {
            addTagPattern(tag);
            }

        return true;
        }

    std::vector<TagMatch> TagExtractor::extract(const std::string& content, const std::string& filePath)
        {
        std::vector<TagMatch> results;
        std::istringstream stream(content);
        std::string line;
        int lineNum = 1;

        while (std::getline(stream, line))
            {
            appendMatchesForLine(line, tags_, lineNum, filePath, results);
            ++lineNum;
            }

        return results;
        }

    std::vector<TagMatch> TagExtractor::extractFromFile(const std::filesystem::path& path, const std::string& filePath)
        {
        std::ifstream input(path);
        if (!input)
            {
            return {};
            }

        std::vector<TagMatch> results;
        std::string line;
        int lineNum = 1;
        while (std::getline(input, line))
            {
            appendMatchesForLine(line, tags_, lineNum, filePath, results);
            ++lineNum;
            }

        return results;
        }
    }
