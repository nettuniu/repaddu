#include "repaddu/cli_parse.h"
#include "repaddu/cli_run.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>

void test_config_path_parsing()
    {
    const std::vector<std::string> args =
        {
            "repaddu",
            "--config",
            "custom.json",
            "--init"
        };

    const auto result = repaddu::cli::parseArgs(args);
    assert(result.result.code == repaddu::core::ExitCode::success);
    assert(result.options.generateConfig == true);
    assert(result.options.configPath == std::filesystem::path("custom.json"));
    }

void test_config_missing_value()
    {
    const std::vector<std::string> args =
        {
            "repaddu",
            "--config"
        };

    const auto result = repaddu::cli::parseArgs(args);
    assert(result.result.code == repaddu::core::ExitCode::invalid_usage);
    }

void test_generate_config_custom_path()
    {
    const std::filesystem::path tempPath = std::filesystem::temp_directory_path()
        / "repaddu_custom_config_test.json";
    std::error_code errorCode;
    std::filesystem::remove(tempPath, errorCode);

    repaddu::core::CliOptions options;
    options.generateConfig = true;
    options.configPath = tempPath;

    const auto runResult = repaddu::cli::run(options, nullptr);
    assert(runResult.code == repaddu::core::ExitCode::success);
    assert(std::filesystem::exists(tempPath));

    std::filesystem::remove(tempPath, errorCode);
    }

int main()
    {
    test_config_path_parsing();
    test_config_missing_value();
    test_generate_config_custom_path();
    std::cout << "CLI config path tests passed." << std::endl;
    return 0;
    }
