#include "repaddu/analysis_tags.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace repaddu::analysis
    {
    namespace
        {
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

        void appendMatchesForLine(const std::string& line,
            const std::vector<std::string>& tags,
            int lineNum,
            const std::string& filePath,
            std::vector<TagMatch>& results)
            {
            for (const auto& tag : tags)
                {
                const std::size_t pos = line.find(tag);
                if (pos == std::string::npos)
                    {
                    continue;
                    }

                TagMatch match;
                match.tag = tag;
                match.lineNumber = lineNum;
                match.filePath = filePath;

                std::size_t contentPos = pos + tag.length();
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
