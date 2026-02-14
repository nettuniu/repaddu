#include "repaddu/app_run.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace
    {
    class MockUi : public repaddu::ui::UserInterface
        {
        public:
            void logInfo(const std::string& msg) override
                {
                infos.push_back(msg);
                }

            void logError(const std::string& msg) override
                {
                errors.push_back(msg);
                }

            void logWarning(const std::string& msg) override
                {
                warnings.push_back(msg);
                }

            void startProgress(const std::string& label, int totalSteps) override
                {
                (void)totalSteps;
                startLabels.push_back(label);
                ++startCount;
                }

            void updateProgress(int step, const std::string& status = "") override
                {
                (void)step;
                (void)status;
                ++updateCount;
                }

            void endProgress() override
                {
                ++endCount;
                }

            std::vector<std::string> infos;
            std::vector<std::string> warnings;
            std::vector<std::string> errors;
            std::vector<std::string> startLabels;
            int startCount = 0;
            int updateCount = 0;
            int endCount = 0;
        };

    void test_progress_lifecycle_on_traversal_failure()
        {
        repaddu::core::CliOptions options;
        options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "README.md";
        options.outputPath = std::filesystem::temp_directory_path() / "repaddu_app_run_out";

        std::error_code errorCode;
        std::filesystem::remove_all(options.outputPath, errorCode);

        MockUi ui;
        const repaddu::core::RunResult result = repaddu::app::run(options, ui);

        assert(result.code != repaddu::core::ExitCode::success);
        assert(ui.startCount == 1);
        assert(ui.endCount == 1);
        assert(ui.startLabels.size() == 1);
        assert(ui.startLabels[0] == "Scanning repository");
        }

    void test_progress_lifecycle_on_analyze_only_success()
        {
        repaddu::core::CliOptions options;
        options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/sample_repo";
        options.outputPath = std::filesystem::temp_directory_path() / "repaddu_app_run_analyze_only_out";
        options.analyzeOnly = true;

        std::error_code errorCode;
        std::filesystem::remove_all(options.outputPath, errorCode);

        MockUi ui;
        const repaddu::core::RunResult result = repaddu::app::run(options, ui);

        assert(result.code == repaddu::core::ExitCode::success);
        assert(ui.startCount == 2);
        assert(ui.endCount == 2);
        assert(ui.startLabels.size() == 2);
        assert(ui.startLabels[0] == "Scanning repository");
        assert(ui.startLabels[1] == "Grouping files");
        }
    }

int main()
    {
    test_progress_lifecycle_on_traversal_failure();
    test_progress_lifecycle_on_analyze_only_success();
    std::cout << "App run tests passed." << std::endl;
    return 0;
    }
