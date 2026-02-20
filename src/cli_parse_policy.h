#ifndef REPADDU_CLI_PARSE_POLICY_H
#define REPADDU_CLI_PARSE_POLICY_H

#include "repaddu/core_types.h"

namespace repaddu::cli::detail
    {
    struct SourceSelectionFlags
        {
        bool includeHeadersExplicit = false;
        bool includeSourcesExplicit = false;
        };

    void mergeSourceSelectionDefaults(core::CliOptions& options, const SourceSelectionFlags& flags);
    core::RunResult validateParsedOptions(const core::CliOptions& options);
    }

#endif // REPADDU_CLI_PARSE_POLICY_H
