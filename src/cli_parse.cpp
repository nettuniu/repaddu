#include "repaddu/cli_parse.h"

#include "repaddu/core_types.h"
#include "repaddu/language_profiles.h"

#include "cli_parse_values.h"

namespace repaddu::cli
    {

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
                if (!detail::parseInt(value, parsed) || parsed < 0)
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
                if (!detail::parseUInt64(value, parsed))
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
                if (!detail::parseInt(value, parsed) || parsed <= 0)
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
                options.extensions = detail::splitCsv(value);
                }
            else if (arg == "--exclude-extensions")
                {
                std::string value;
                if (!requireValue(value))
                    {
                    return { options, { core::ExitCode::invalid_usage, "--exclude-extensions requires a value." }, "" };
                    }
                options.excludeExtensions = detail::splitCsv(value);
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
                if (!detail::parseInt(value, parsed) || parsed <= 0)
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
                if (!detail::parseUInt64(value, parsed))
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
                options.analysisViews = detail::splitCsv(value);
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
            else if (arg == "--token-count")
                {
                // Token estimation is already part of output/analysis paths.
                // Keep this flag as a compatibility alias for documented scenarios.
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
    }
