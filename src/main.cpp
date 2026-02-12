#include "repaddu/cli_parse.h"
#include "repaddu/cli_run.h"
#include "repaddu/ui_console.h"

#include <iostream>
#include <vector>
#include <filesystem>

namespace
    {
    std::filesystem::path detectConfigPathFromArgs(const std::vector<std::string>& args)
        {
        for (std::size_t i = 1; i + 1 < args.size(); ++i)
            {
            if (args[i] == "--config")
                {
                return args[i + 1];
                }
            }
        return ".repaddu.json";
        }
    }

int main(int argc, char** argv)
    {
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i)
        {
        args.emplace_back(argv[i]);
        }

    repaddu::core::CliOptions options;

    // 1. Try to load config from file
    std::filesystem::path configPath = detectConfigPathFromArgs(args);
    options.configPath = configPath;
    if (std::filesystem::exists(configPath))
        {
        repaddu::cli::loadConfigFile(configPath, options);
        }

    // 2. Parse args (overwrites config)
    repaddu::cli::ParseResult parseResult = repaddu::cli::parseArgs(args, options);
    
    if (!parseResult.output.empty())
        {
        std::cout << parseResult.output;
        return static_cast<int>(parseResult.result.code);
        }

    if (parseResult.result.code != repaddu::core::ExitCode::success)
        {
        std::cerr << parseResult.result.message << "\n";
        std::cerr << repaddu::cli::helpText();
        return static_cast<int>(parseResult.result.code);
        }

    repaddu::ui::ConsoleUI ui;
    repaddu::core::RunResult runResult = repaddu::cli::run(parseResult.options, &ui);
    
    if (runResult.code != repaddu::core::ExitCode::success)
        {
        std::cerr << runResult.message << "\n";
        return static_cast<int>(runResult.code);
        }

    return static_cast<int>(repaddu::core::ExitCode::success);
    }
