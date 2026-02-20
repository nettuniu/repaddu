#include "cli_parse_policy.h"

namespace repaddu::cli::detail
    {
    void mergeSourceSelectionDefaults(core::CliOptions& options, const SourceSelectionFlags& flags)
        {
        if (flags.includeHeadersExplicit && !flags.includeSourcesExplicit)
            {
            options.includeSources = false;
            }
        if (!flags.includeHeadersExplicit && !flags.includeSourcesExplicit)
            {
            options.includeSources = true;
            }
        }

    core::RunResult validateParsedOptions(const core::CliOptions& options)
        {
        if (options.inputPath.empty() && !options.generateConfig)
            {
            return { core::ExitCode::invalid_usage, "--input is required." };
            }
        if (!options.scanLanguages && !options.analyzeOnly && !options.generateConfig && options.outputPath.empty())
            {
            return { core::ExitCode::invalid_usage, "--output is required unless --scan-languages, --analyze-only, or --init is used." };
            }
        if (options.groupBy == core::GroupingMode::component && options.componentMapPath.empty())
            {
            return { core::ExitCode::invalid_usage, "--group-by component requires --component-map." };
            }
        return { core::ExitCode::success, "" };
        }
    }
