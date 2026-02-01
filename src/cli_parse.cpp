#include "repaddu/cli_parse.h"

#include "repaddu/core_types.h"

#include <sstream>

namespace repaddu::cli
    {
    namespace
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

    ParseResult parseArgs(const std::vector<std::string>& args)
        {
        ParseResult result;
        core::CliOptions options;
        bool includeHeadersFlag = false;
        bool includeSourcesFlag = false;

        for (std::size_t i = 1; i < args.size(); ++i)
            {
            const std::string& arg = args[i];
            auto requireValue = [&args, &i](std::string& outValue) -> bool
                {
                if (i + 1 >= args.size())
                    {
                    return false;
                    }
                outValue = args[++i];
                return true;
                };

            if (arg == "-h" || arg == "--help")
                {
                options.showHelp = true;
                }
            else if (arg == "--version")
                {
                options.showVersion = true;
                }
            else if (arg == "-i" || arg == "--input")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--input requires a value." }, "" };
                    }
                options.inputPath = value;
                }
            else if (arg == "-o" || arg == "--output")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--output requires a value." }, "" };
                    }
                options.outputPath = value;
                }
            else if (arg == "--max-files")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--max-files requires a value." }, "" };
                    }
                int parsed = 0;
                if (!parseInt(value, parsed) || parsed < 0)
                    {
                    return { options, { core::ExitCode::invalid_usage, "--max-files must be a non-negative integer." }, "" };
                    }
                options.maxFiles = parsed;
                }
            else if (arg == "--max-bytes")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--max-bytes requires a value." }, "" };
                    }
                std::uintmax_t parsed = 0;
                if (!parseUInt64(value, parsed))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--max-bytes must be a non-negative integer." }, "" };
                    }
                options.maxBytes = parsed;
                }
            else if (arg == "--number-width")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--number-width requires a value." }, "" };
                    }
                int parsed = 0;
                if (!parseInt(value, parsed) || parsed <= 0)
                    {
                    return { options, { core::ExitCode::invalid_usage, "--number-width must be a positive integer." }, "" };
                    }
                options.numberWidth = parsed;
                }
            else if (arg == "--include-headers")
                {
                options.includeHeaders = true;
                includeHeadersFlag = true;
                }
            else if (arg == "--include-sources")
                {
                options.includeSources = true;
                includeSourcesFlag = true;
                }
            else if (arg == "--extensions")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--extensions requires a value." }, "" };
                    }
                options.extensions = splitCsv(value);
                }
            else if (arg == "--exclude-extensions")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--exclude-extensions requires a value." }, "" };
                    }
                options.excludeExtensions = splitCsv(value);
                }
            else if (arg == "--include-hidden")
                {
                options.includeHidden = true;
                }
            else if (arg == "--follow-symlinks")
                {
                options.followSymlinks = true;
                }
            else if (arg == "--include-binaries")
                {
                options.includeBinaries = true;
                }
            else if (arg == "--group-by")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--group-by requires a value." }, "" };
                    }
                if (value == "directory")
                    {
                    options.groupBy = core::GroupingMode::directory;
                    }
                else if (value == "component")
                    {
                    options.groupBy = core::GroupingMode::component;
                    }
                else if (value == "type")
                    {
                    options.groupBy = core::GroupingMode::type;
                    }
                else if (value == "size")
                    {
                    options.groupBy = core::GroupingMode::size;
                    }
                else
                    {
                    return { options, { core::ExitCode::invalid_usage, "--group-by must be one of: directory, component, type, size." }, "" };
                    }
                }
            else if (arg == "--group-depth")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--group-depth requires a value." }, "" };
                    }
                int parsed = 0;
                if (!parseInt(value, parsed) || parsed <= 0)
                    {
                    return { options, { core::ExitCode::invalid_usage, "--group-depth must be a positive integer." }, "" };
                    }
                options.groupDepth = parsed;
                }
            else if (arg == "--component-map")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--component-map requires a value." }, "" };
                    }
                options.componentMapPath = value;
                }
            else if (arg == "--headers-first")
                {
                options.headersFirst = true;
                }
            else if (arg == "--emit-tree")
                {
                options.emitTree = true;
                }
            else if (arg == "--emit-cmake")
                {
                options.emitCMake = true;
                }
            else if (arg == "--markers")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--markers requires a value." }, "" };
                    }
                if (value == "fenced")
                    {
                    options.markers = core::MarkerMode::fenced;
                    }
                else if (value == "sentinel")
                    {
                    options.markers = core::MarkerMode::sentinel;
                    }
                else
                    {
                    return { options, { core::ExitCode::invalid_usage, "--markers must be one of: fenced, sentinel." }, "" };
                    }
                }
            else if (arg == "--scan-languages")
                {
                options.scanLanguages = true;
                }
            else
                {
                return { options, { core::ExitCode::invalid_usage, "Unknown argument: " + arg }, "" };
                }
            }

        if (includeHeadersFlag && !includeSourcesFlag)
            {
            options.includeSources = false;
            }
        if (!includeHeadersFlag && !includeSourcesFlag)
            {
            options.includeSources = true;
            }

        if (options.showHelp)
            {
            result.options = options;
            result.output = helpText();
            result.result = { core::ExitCode::success, "" };
            return result;
            }

        if (options.showVersion)
            {
            result.options = options;
            result.output = versionText();
            result.result = { core::ExitCode::success, "" };
            return result;
            }

        if (options.inputPath.empty())
            {
            return { options, { core::ExitCode::invalid_usage, "--input is required." }, "" };
            }
        if (!options.scanLanguages && options.outputPath.empty())
            {
            return { options, { core::ExitCode::invalid_usage, "--output is required unless --scan-languages is used." }, "" };
            }

        if (options.groupBy == core::GroupingMode::component && options.componentMapPath.empty())
            {
            return { options, { core::ExitCode::invalid_usage, "--group-by component requires --component-map." }, "" };
            }

        result.options = options;
        result.result = { core::ExitCode::success, "" };
        return result;
        }

    std::string helpText()
        {
        std::ostringstream out;
        out << "repaddu - convert a repository/folder into numbered Markdown outputs\n\n";
        out << "Usage:\n";
        out << "  repaddu [options] --input <path> --output <path>\n\n";
        out << "Options:\n";
        out << "  -i, --input <path>          Input repository/folder path.\n";
        out << "  -o, --output <path>         Output directory.\n";
        out << "  --max-files <count>         Maximum number of output files. Default: 0.\n";
        out << "  --max-bytes <bytes>         Maximum bytes per output file. Default: 0.\n";
        out << "  --number-width <n>          Width of numeric prefix. Default: 3.\n";
        out << "  --include-headers           Include only headers (.h/.hpp/.hh/.hxx).\n";
        out << "  --include-sources           Include sources (.c/.cc/.cpp/.cxx).\n";
        out << "  --extensions <csv>          Override include list with explicit extensions.\n";
        out << "  --exclude-extensions <csv>  Exclude these extensions after includes.\n";
        out << "  --include-hidden            Include hidden files/directories.\n";
        out << "  --follow-symlinks           Follow directory symlinks.\n";
        out << "  --include-binaries          Include binary files.\n";
        out << "  --group-by <mode>           directory|component|type|size. Default: directory.\n";
        out << "  --group-depth <n>           Depth for directory grouping. Default: 1.\n";
        out << "  --component-map <path>      JSON component mapping file for component grouping.\n";
        out << "  --headers-first             Order headers before sources in groups.\n";
        out << "  --emit-tree                 Emit recursive tree listing.\n";
        out << "  --emit-cmake                Emit aggregated CMakeLists.txt output.\n";
        out << "  --markers <mode>            fenced|sentinel. Default: fenced.\n";
        out << "  --scan-languages            Scan repository and report language percentages only.\n";
        out << "  -h, --help                  Show help.\n";
        out << "  --version                   Show version.\n";
        return out.str();
        }

    std::string versionText()
        {
        return "repaddu 0.1.0\n";
        }
    }
