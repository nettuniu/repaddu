#include "repaddu/cli_parse.h"
#include "repaddu/cli_run.h"

#include <iostream>
#include <vector>

int main(int argc, char** argv)
    {
    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i)
        {
        args.emplace_back(argv[i]);
        }

    repaddu::cli::ParseResult parseResult = repaddu::cli::parseArgs(args);
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

    repaddu::core::RunResult runResult = repaddu::cli::run(parseResult.options);
    if (runResult.code != repaddu::core::ExitCode::success)
        {
        std::cerr << runResult.message << "\n";
        return static_cast<int>(runResult.code);
        }

    return static_cast<int>(repaddu::core::ExitCode::success);
    }
