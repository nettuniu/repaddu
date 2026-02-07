#ifndef REPADDU_CORE_TYPES_H
#define REPADDU_CORE_TYPES_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace repaddu::core
    {
    enum class ExitCode : int
        {
        success = 0,
        invalid_usage = 1,
        io_failure = 2,
        traversal_failure = 3,
        output_constraints = 4
        };

    enum class GroupingMode
        {
        directory,
        component,
        type,
        size
        };

    enum class MarkerMode
        {
        fenced,
        sentinel
        };

    enum class FileClass
        {
        header,
        source,
        other
        };

    enum class OutputFormat
        {
        markdown,
        jsonl,
        html
        };

    struct CliOptions
        {
        std::filesystem::path inputPath;
        std::filesystem::path outputPath;
        int maxFiles = 0;
        std::uintmax_t maxBytes = 0;
        int numberWidth = 3;
        bool includeHeaders = false;
        bool includeSources = true;
        std::vector<std::string> extensions;
        std::vector<std::string> excludeExtensions;
        bool includeHidden = false;
        bool followSymlinks = false;
        bool includeBinaries = false;
        GroupingMode groupBy = GroupingMode::directory;
        int groupDepth = 1;
        std::filesystem::path componentMapPath;
        bool headersFirst = false;
        bool emitTree = true;
        bool emitCMake = true;
        bool emitBuildFiles = false;
        MarkerMode markers = MarkerMode::fenced;
        OutputFormat format = OutputFormat::markdown;
        bool showHelp = false;
        bool showVersion = false;
        bool scanLanguages = false;
        std::string language;
        std::string buildSystem;

        std::uintmax_t maxFileSize = 1024 * 1024; // 1MB default
        bool forceLargeFiles = false;
        bool redactPii = false;
        bool analyzeOnly = false;
        bool isolateDocs = false;
        bool dryRun = false;
        bool generateConfig = false;
        };

    struct FileEntry
        {
        std::filesystem::path absolutePath;
        std::filesystem::path relativePath;
        std::string extensionLower;
        std::uintmax_t sizeBytes = 0;
        std::uintmax_t tokenCount = 0;
        bool isBinary = false;
        FileClass fileClass = FileClass::other;
        };

    struct Group
        {
        std::string name;
        std::vector<std::size_t> fileIndices;
        };

    struct OutputChunk
        {
        std::string category;
        std::string title;
        std::vector<std::size_t> fileIndices;
        };

    struct OutputContent
        {
        std::string filename;
        std::string content;
        std::uintmax_t contentBytes = 0;
        };

    struct RunResult
        {
        ExitCode code = ExitCode::success;
        std::string message;
        };

    std::string toLowerCopy(std::string_view value);
    std::string sanitizeName(std::string_view value);
    std::string fileClassLabel(FileClass value);
    FileClass classifyExtension(std::string_view extensionLower);
    }

#endif // REPADDU_CORE_TYPES_H
