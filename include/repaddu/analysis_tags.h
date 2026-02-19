#ifndef REPADDU_ANALYSIS_TAGS_H
#define REPADDU_ANALYSIS_TAGS_H

#include <filesystem>
#include <string>
#include <vector>

namespace repaddu::analysis
    {
    struct TagMatch
        {
        std::string tag;
        std::string content;
        int lineNumber;
        std::string filePath;
        };

    class TagExtractor
        {
        public:
            TagExtractor();

            // Adds a custom tag pattern to look for
            void addTagPattern(const std::string& tag);
            bool loadTagPatternsFromFile(const std::filesystem::path& path, bool replaceExisting = false);

            // Scans content and returns matches
            std::vector<TagMatch> extract(const std::string& content, const std::string& filePath = "");
            std::vector<TagMatch> extractFromFile(const std::filesystem::path& path, const std::string& filePath = "");

        private:
            std::vector<std::string> tags_;
        };
    }

#endif // REPADDU_ANALYSIS_TAGS_H
