#include "repaddu/analysis_tags.h"
#include <sstream>
#include <algorithm>

namespace repaddu::analysis
    {
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
