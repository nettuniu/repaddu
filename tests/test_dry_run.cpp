#include "repaddu/cli_run.h"

#include <cassert>
#include <filesystem>
#include <iostream>

void test_dry_run_writes_no_files()
    {
    const std::filesystem::path outPath = std::filesystem::temp_directory_path() / "repaddu_dry_run_out";
    std::error_code errorCode;
    std::filesystem::remove_all(outPath, errorCode);

    repaddu::core::CliOptions options;
    options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/sample_repo";
    options.outputPath = outPath;
    options.dryRun = true;

    const auto result = repaddu::cli::run(options, nullptr);
    assert(result.code == repaddu::core::ExitCode::success);

    bool hasFiles = false;
    if (std::filesystem::exists(outPath))
        {
        for (const auto& entry : std::filesystem::directory_iterator(outPath))
            {
            (void)entry;
            hasFiles = true;
            break;
            }
        }

    assert(hasFiles == false);

    std::filesystem::remove_all(outPath, errorCode);
    }

int main()
    {
    test_dry_run_writes_no_files();
    std::cout << "Dry-run tests passed." << std::endl;
    return 0;
    }
