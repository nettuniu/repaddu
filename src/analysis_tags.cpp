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
            for (const auto& tag : tags_)
                {
                // Simple search for "TAG:" or "TAG " or "@TAG"
                // We want to be flexible but avoid false positives.
                // Case insensitive search might be better, but standard C++ doesn't have it easily for strings.
                // For now, let's do a simple case-sensitive search.
                
                size_t pos = line.find(tag);
                if (pos != std::string::npos)
                    {
                    // Found a potential match
                    TagMatch match;
                    match.tag = tag;
                    match.lineNumber = lineNum;
                    match.filePath = filePath;
                    
                    // Extract content after the tag
                    size_t contentPos = pos + tag.length();
                    // Skip colon or spaces
                    while (contentPos < line.length() && (line[contentPos] == ':' || std::isspace(line[contentPos])))
                        {
                        contentPos++;
                        }
                    
                    match.content = line.substr(contentPos);
                    // Trim trailing whitespace
                    size_t last = match.content.find_last_not_of(" \t\n\r");
                    if (last != std::string::npos)
                        {
                        match.content = match.content.substr(0, last + 1);
                        }

                    results.push_back(std::move(match));
                    }
                }
            lineNum++;
            }

        return results;
        }
    }
