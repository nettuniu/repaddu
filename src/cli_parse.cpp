#include "repaddu/cli_parse.h"

#include "repaddu/core_types.h"
#include "repaddu/language_profiles.h"
#include "repaddu/logger.h"
#include "repaddu/json_lite.h"

#include <cctype>
#include <fstream>
#include <map>
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

    ParseResult parseArgs(const std::vector<std::string>& args, const core::CliOptions& baseOptions)
        {
        ParseResult result;
        core::CliOptions options = baseOptions;
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
            else if (arg == "--config")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--config requires a value." }, "" };
                    }
                options.configPath = value;
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
            else if (arg == "--single-thread")
                {
                options.parallelTraversal = false;
                }
            else if (arg == "--parallel-traversal")
                {
                options.parallelTraversal = true;
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
            else if (arg == "--frontmatter")
                {
                options.emitFrontmatter = true;
                }
            else if (arg == "--scan-languages")
                {
                options.scanLanguages = true;
                }
            else if (arg == "--language")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--language requires a value." }, "" };
                    }
                if (value != "auto" && core::findLanguageProfile(value) == nullptr)
                    {
                    return { options, { core::ExitCode::invalid_usage, "--language must be one of: auto, c, cpp, rust, python." }, "" };
                    }
                options.language = (value == "auto") ? "" : value;
                }
            else if (arg == "--build-system")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--build-system requires a value." }, "" };
                    }
                if (value != "auto" && core::findBuildSystemProfile(value) == nullptr)
                    {
                    return { options, { core::ExitCode::invalid_usage, "--build-system must be one of: auto, cmake, make, meson, bazel, cargo, npm, python." }, "" };
                    }
                options.buildSystem = (value == "auto") ? "" : value;
                }
            else if (arg == "--emit-build-files")
                {
                options.emitBuildFiles = true;
                }
            else if (arg == "--no-links")
                {
                options.emitLinks = false;
                }
            else if (arg == "--max-file-size")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--max-file-size requires a value." }, "" };
                    }
                std::uintmax_t parsed = 0;
                if (!parseUInt64(value, parsed))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--max-file-size must be a non-negative integer." }, "" };
                    }
                options.maxFileSize = parsed;
                }
            else if (arg == "--force-large")
                {
                options.forceLargeFiles = true;
                }
            else if (arg == "--redact-pii")
                {
                options.redactPii = true;
                }
            else if (arg == "--analyze-only")
                {
                options.analyzeOnly = true;
                }
            else if (arg == "--analysis")
                {
                options.analysisEnabled = true;
                }
            else if (arg == "--analysis-views")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--analysis-views requires a value." }, "" };
                    }
                options.analysisViews = splitCsv(value);
                }
            else if (arg == "--analysis-deep")
                {
                options.analysisDeep = true;
                }
            else if (arg == "--analysis-collapse")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--analysis-collapse requires a value." }, "" };
                    }
                if (value == "none" || value == "folder" || value == "target")
                    {
                    options.analysisCollapse = value;
                    }
                else
                    {
                    return { options, { core::ExitCode::invalid_usage, "--analysis-collapse must be one of: none, folder, target." }, "" };
                    }
                }
            else if (arg == "--extract-tags")
                {
                options.extractTags = true;
                }
            else if (arg == "--tag-patterns")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--tag-patterns requires a value." }, "" };
                    }
                options.tagPatternsPath = value;
                }
            else if (arg == "--isolate-docs")
                {
                options.isolateDocs = true;
                }
            else if (arg == "--dry-run")
                {
                options.dryRun = true;
                }
            else if (arg == "--init" || arg == "--generate-config")
                {
                options.generateConfig = true;
                }
            else if (arg == "--format")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--format requires a value." }, "" };
                    }
                if (value == "markdown")
                    {
                    options.format = core::OutputFormat::markdown;
                    }
                else if (value == "jsonl")
                    {
                    options.format = core::OutputFormat::jsonl;
                    }
                else if (value == "html")
                    {
                    options.format = core::OutputFormat::html;
                    }
                else
                    {
                    return { options, { core::ExitCode::invalid_usage, "--format must be one of: markdown, jsonl, html." }, "" };
                    }
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

        if (options.inputPath.empty() && !options.generateConfig)
            {
            return { options, { core::ExitCode::invalid_usage, "--input is required." }, "" };
            }
        if (!options.scanLanguages && !options.analyzeOnly && !options.generateConfig && options.outputPath.empty())
            {
            return { options, { core::ExitCode::invalid_usage, "--output is required unless --scan-languages, --analyze-only, or --init is used." }, "" };
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
        out << "  --single-thread             Force single-threaded traversal.\n";
        out << "  --parallel-traversal        Enable parallel traversal (default).\n";
        out << "  --include-binaries          Include binary files.\n";
        out << "  --max-file-size <bytes>     Skip files larger than this (default 1MB).\n";
        out << "  --force-large               Include large files despite size check.\n";
        out << "  --redact-pii                Redact emails, IPs, and secrets from output.\n";
        out << "  --analyze-only              Scan and report statistics without generating files.\n";
        out << "  --analysis                  Enable symbol analysis (AST/LSP) when available.\n";
        out << "  --analysis-views <csv>      Comma-separated analysis views to emit.\n";
        out << "  --analysis-deep             Enable deeper relationship analysis (optional edges).\n";
        out << "  --analysis-collapse <mode>  none|folder|target. Default: none.\n";
        out << "  --extract-tags              Extract TODO/FIXME-like tags in analyze output.\n";
        out << "  --tag-patterns <path>       Load additional tag patterns from file (one per line).\n";
        out << "  --isolate-docs              Group all documentation files (*.md, *.txt) into a separate chunk.\n";
        out << "  --dry-run                   Simulate execution without writing files.\n";
        out << "  --init                      Generate a default config file (JSON or YAML by --config extension).\n";
        out << "  --config <path>             Config path to load and/or generate. Default: .repaddu.json.\n";
        out << "  --format <fmt>              markdown|jsonl|html. Default: markdown.\n";
        out << "  --group-by <mode>           directory|component|type|size. Default: directory.\n";
        out << "  --group-depth <n>           Depth for directory grouping. Default: 1.\n";
        out << "  --component-map <path>      JSON component mapping file for component grouping.\n";
        out << "  --headers-first             Order headers before sources in groups.\n";
        out << "  --emit-tree                 Emit recursive tree listing.\n";
        out << "  --emit-cmake                Emit aggregated CMakeLists.txt output.\n";
        out << "  --emit-build-files          Emit aggregated build-system files.\n";
        out << "  --no-links                  Disable markdown links in overview table of contents.\n";
        out << "  --markers <mode>            fenced|sentinel. Default: fenced.\n";
        out << "  --frontmatter               Add YAML frontmatter metadata before each file content block.\n";
        out << "  --scan-languages            Scan repository and report language percentages only.\n";
        out << "  --language <id>             auto|c|cpp|rust|python. Default: auto.\n";
        out << "  --build-system <id>         auto|cmake|make|meson|bazel|cargo|npm|python. Default: auto.\n";
        out << "  -h, --help                  Show help.\n";
        out << "  --version                   Show version.\n";
        return out.str();
        }

    std::string versionText()
        {
        return "repaddu 0.1.0\n";
        }

    core::RunResult loadConfigFile(const std::filesystem::path& path, core::CliOptions& opt)
        {
        std::ifstream ifs(path);
        if (!ifs)
            {
            return { core::ExitCode::io_failure, "Failed to open config file: " + path.string() };
            }

        std::ostringstream buffer;
        buffer << ifs.rdbuf();

        auto trim = [](const std::string& value)
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
            };

        auto unquote = [&](const std::string& value)
            {
            if (value.size() >= 2
                && ((value.front() == '"' && value.back() == '"')
                    || (value.front() == '\'' && value.back() == '\'')))
                {
                return value.substr(1, value.size() - 2);
                }
            return value;
            };

        auto parseConfigCommon = [&](auto&& getBool, auto&& getString, auto&& getPath,
                                     auto&& getInt, auto&& getUInt64, auto&& getStringArray)
            {
            getPath("input", opt.inputPath);
            getPath("output", opt.outputPath);
            getInt("max_files", opt.maxFiles);
            getUInt64("max_bytes", opt.maxBytes);
            getInt("number_width", opt.numberWidth);
            getBool("include_headers", opt.includeHeaders);
            getBool("include_sources", opt.includeSources);
            getBool("include_hidden", opt.includeHidden);
            getBool("include_binaries", opt.includeBinaries);
            getBool("follow_symlinks", opt.followSymlinks);
            getBool("headers_first", opt.headersFirst);
            getBool("emit_tree", opt.emitTree);
            getBool("emit_cmake", opt.emitCMake);
            getBool("emit_build_files", opt.emitBuildFiles);
            getBool("emit_links", opt.emitLinks);
            getBool("frontmatter", opt.emitFrontmatter);
            getUInt64("max_file_size", opt.maxFileSize);
            getBool("force_large", opt.forceLargeFiles);
            getBool("redact_pii", opt.redactPii);
            getBool("analyze_only", opt.analyzeOnly);
            getBool("analysis_enabled", opt.analysisEnabled);
            getString("analysis_collapse", opt.analysisCollapse);
            getBool("analysis_deep", opt.analysisDeep);
            getStringArray("analysis_views", opt.analysisViews);
            getBool("extract_tags", opt.extractTags);
            getPath("tag_patterns", opt.tagPatternsPath);
            getBool("isolate_docs", opt.isolateDocs);
            getBool("dry_run", opt.dryRun);
            getBool("parallel_traversal", opt.parallelTraversal);
            getStringArray("extensions", opt.extensions);
            getStringArray("exclude_extensions", opt.excludeExtensions);

            std::string value;
            getString("group_by", value);
            if (value == "directory") opt.groupBy = core::GroupingMode::directory;
            else if (value == "component") opt.groupBy = core::GroupingMode::component;
            else if (value == "type") opt.groupBy = core::GroupingMode::type;
            else if (value == "size") opt.groupBy = core::GroupingMode::size;

            value.clear();
            getString("format", value);
            if (value == "markdown") opt.format = core::OutputFormat::markdown;
            else if (value == "jsonl") opt.format = core::OutputFormat::jsonl;
            else if (value == "html") opt.format = core::OutputFormat::html;

            value.clear();
            getString("markers", value);
            if (value == "fenced") opt.markers = core::MarkerMode::fenced;
            else if (value == "sentinel") opt.markers = core::MarkerMode::sentinel;
            };

        const std::string extension = core::toLowerCopy(path.extension().string());
        if (extension == ".yaml" || extension == ".yml")
            {
            std::map<std::string, std::string> scalars;
            std::map<std::string, std::vector<std::string>> arrays;

            std::istringstream stream(buffer.str());
            std::string line;
            while (std::getline(stream, line))
                {
                std::string noInlineComment;
                noInlineComment.reserve(line.size());
                bool inSingleQuote = false;
                bool inDoubleQuote = false;
                bool escapedInDoubleQuote = false;
                for (char ch : line)
                    {
                    if (ch == '\\' && inDoubleQuote && !escapedInDoubleQuote)
                        {
                        escapedInDoubleQuote = true;
                        noInlineComment.push_back(ch);
                        continue;
                        }

                    if (ch == '\'' && !inDoubleQuote)
                        {
                        inSingleQuote = !inSingleQuote;
                        }
                    else if (ch == '"' && !inSingleQuote && !escapedInDoubleQuote)
                        {
                        inDoubleQuote = !inDoubleQuote;
                        }
                    else if (ch == '#' && !inSingleQuote && !inDoubleQuote)
                        {
                        break;
                        }
                    noInlineComment.push_back(ch);
                    escapedInDoubleQuote = false;
                    }

                const std::string stripped = trim(noInlineComment);
                if (stripped.empty() || stripped.front() == '#')
                    {
                    continue;
                    }

                const std::size_t colon = stripped.find(':');
                if (colon == std::string::npos)
                    {
                    continue;
                    }

                const std::string key = trim(stripped.substr(0, colon));
                std::string value = trim(stripped.substr(colon + 1));
                if (value.empty())
                    {
                    scalars[key] = "";
                    continue;
                    }

                if (value.front() == '[' && value.back() == ']')
                    {
                    std::string inner = trim(value.substr(1, value.size() - 2));
                    std::vector<std::string> items;
                    std::istringstream itemStream(inner);
                    std::string item;
                    while (std::getline(itemStream, item, ','))
                        {
                        const std::string trimmed = trim(item);
                        if (trimmed.empty())
                            {
                            continue;
                            }
                        items.push_back(unquote(trimmed));
                        }
                    arrays[key] = items;
                    }
                else
                    {
                    scalars[key] = unquote(value);
                    }
                }

            auto getBool = [&](const std::string& key, bool& target)
                {
                auto it = scalars.find(key);
                if (it == scalars.end())
                    {
                    return;
                    }
                const std::string lowered = core::toLowerCopy(it->second);
                if (lowered == "true" || lowered == "1" || lowered == "yes")
                    {
                    target = true;
                    }
                else if (lowered == "false" || lowered == "0" || lowered == "no")
                    {
                    target = false;
                    }
                };
            auto getString = [&](const std::string& key, std::string& target)
                {
                auto it = scalars.find(key);
                if (it != scalars.end())
                    {
                    target = it->second;
                    }
                };
            auto getPath = [&](const std::string& key, std::filesystem::path& target)
                {
                auto it = scalars.find(key);
                if (it != scalars.end())
                    {
                    target = it->second;
                    }
                };
            auto getInt = [&](const std::string& key, int& target)
                {
                auto it = scalars.find(key);
                if (it == scalars.end())
                    {
                    return;
                    }
                try
                    {
                    target = std::stoi(it->second);
                    }
                catch (...)
                    {
                    }
                };
            auto getUInt64 = [&](const std::string& key, std::uintmax_t& target)
                {
                auto it = scalars.find(key);
                if (it == scalars.end())
                    {
                    return;
                    }
                try
                    {
                    target = static_cast<std::uintmax_t>(std::stoull(it->second));
                    }
                catch (...)
                    {
                    }
                };
            auto getStringArray = [&](const std::string& key, std::vector<std::string>& target)
                {
                auto it = arrays.find(key);
                if (it != arrays.end())
                    {
                    target = it->second;
                    }
                };

            parseConfigCommon(getBool, getString, getPath, getInt, getUInt64, getStringArray);
            return { core::ExitCode::success, "" };
            }

        json::JsonValue root;
        core::RunResult parseRes = json::parse(buffer.str(), root);
        if (parseRes.code != core::ExitCode::success)
            {
            return parseRes;
            }

        if (!root.isObject())
            {
            return { core::ExitCode::invalid_usage, "Config file must be a JSON object." };
            }

        const auto& obj = root.getObject();
        auto getBool = [&](const std::string& key, bool& target)
            {
            if (obj.count(key) && obj.at(key).isBool())
                {
                target = obj.at(key).getBool();
                }
            };
        auto getString = [&](const std::string& key, std::string& target)
            {
            if (obj.count(key) && obj.at(key).isString())
                {
                target = obj.at(key).getString();
                }
            };
        auto getPath = [&](const std::string& key, std::filesystem::path& target)
            {
            if (obj.count(key) && obj.at(key).isString())
                {
                target = obj.at(key).getString();
                }
            };
        auto getInt = [&](const std::string& key, int& target)
            {
            if (obj.count(key) && obj.at(key).isNumber())
                {
                target = static_cast<int>(obj.at(key).getNumber());
                }
            };
        auto getUInt64 = [&](const std::string& key, std::uintmax_t& target)
            {
            if (obj.count(key) && obj.at(key).isNumber())
                {
                target = static_cast<std::uintmax_t>(obj.at(key).getNumber());
                }
            };
        auto getStringArray = [&](const std::string& key, std::vector<std::string>& target)
            {
            if (obj.count(key) && obj.at(key).isArray())
                {
                target.clear();
                for (const auto& value : obj.at(key).getArray())
                    {
                    if (value.isString())
                        {
                        target.push_back(value.getString());
                        }
                    }
                }
            };

        parseConfigCommon(getBool, getString, getPath, getInt, getUInt64, getStringArray);
        return { core::ExitCode::success, "" };
        }
    }
