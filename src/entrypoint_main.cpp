#include "repaddu/entrypoint_main.h"

#include "repaddu/cli_bootstrap.h"
#include "repaddu/cli_parse.h"
#include "repaddu/cli_run.h"

#include <iostream>

namespace repaddu::entrypoint
    {
    int runMain(const std::vector<std::string>& args)
        {
        cli::ParseResult parseResult = cli::parseArgsWithConfig(args);

        if (!parseResult.output.empty())
            {
            std::cout << parseResult.output;
            return static_cast<int>(parseResult.result.code);
            }

        if (parseResult.result.code != core::ExitCode::success)
            {
            std::cerr << parseResult.result.message << "\n";
            std::cerr << cli::helpText();
            return static_cast<int>(parseResult.result.code);
            }

        const core::RunResult runResult = cli::run(parseResult.options);
        if (runResult.code != core::ExitCode::success)
            {
            std::cerr << runResult.message << "\n";
            return static_cast<int>(runResult.code);
            }

        return static_cast<int>(core::ExitCode::success);
        }
    }
