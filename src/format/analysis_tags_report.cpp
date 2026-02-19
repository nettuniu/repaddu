#include "repaddu/format/analysis_tags_report.h"

#include "repaddu/analysis_tags.h"

#include <map>
#include <sstream>

namespace repaddu::format
    {
    std::string renderTagSummaryReport(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const std::vector<std::size_t>& includedIndices)
        {
        analysis::TagExtractor extractor;
        if (!options.tagPatternsPath.empty())
            {
            extractor.loadTagPatternsFromFile(options.tagPatternsPath, false);
            }

        std::map<std::string, std::size_t> counts;
        std::vector<analysis::TagMatch> matches;
        for (std::size_t index : includedIndices)
            {
            const auto& entry = files[index];
            if (entry.isBinary)
                {
                continue;
                }

            const auto fileMatches = extractor.extractFromFile(entry.absolutePath, entry.relativePath.string());
            for (const auto& match : fileMatches)
                {
                counts[match.tag] += 1;
                matches.push_back(match);
                }
            }

        std::ostringstream out;
        out << "\nTAG SUMMARY\n";
        out << "===========\n";
        if (matches.empty())
            {
            out << "No matching tags found.\n";
            return out.str();
            }

        out << "Counts:\n";
        for (const auto& entry : counts)
            {
            out << "- " << entry.first << ": " << entry.second << "\n";
            }

        out << "\nMatches:\n";
        for (const auto& match : matches)
            {
            out << "- " << match.filePath << ":" << match.lineNumber
                << " [" << match.tag << "] " << match.content << "\n";
            }
        return out.str();
        }
    }
