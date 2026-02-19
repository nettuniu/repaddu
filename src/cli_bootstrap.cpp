#include "repaddu/cli_bootstrap.h"

#include <filesystem>

namespace repaddu::cli
    {
    ParseResult parseArgsWithConfig(const std::vector<std::string>& args)
        {
        core::CliOptions options;

        const std::filesystem::path configPath = resolveConfigPath(args);
        options.configPath = configPath;
        if (std::filesystem::exists(configPath))
            {
            // Keep historical behavior: best-effort load before CLI parsing.
            loadConfigFile(configPath, options);
            }

        return parseArgs(args, options);
        }
    }
