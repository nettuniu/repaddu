#ifndef REPADDU_CLI_PARSE_H
#define REPADDU_CLI_PARSE_H

#include "repaddu/core_types.h"

#include <vector>

namespace repaddu::cli
    {
        struct ParseResult
            {
            core::CliOptions options;
            core::RunResult result;
            std::string output;
            };
    
        ParseResult parseArgs(const std::vector<std::string>& args, const core::CliOptions& baseOptions = core::CliOptions{});
        std::filesystem::path resolveConfigPath(const std::vector<std::string>& args);
        
        core::RunResult loadConfigFile(const std::filesystem::path& path, core::CliOptions& outOptions);
    
        std::string helpText();
        std::string versionText();
        }

#endif // REPADDU_CLI_PARSE_H
