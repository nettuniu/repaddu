#include "repaddu/cli_run.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <sstream>

void test_text_analysis_output()
    {
    repaddu::core::CliOptions options;
    options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/sample_repo";
    options.outputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/out";
    options.analyzeOnly = true;
    options.analysisEnabled = true;

    std::ostringstream captured;
    std::streambuf* oldBuffer = std::cout.rdbuf(captured.rdbuf());
    const auto result = repaddu::cli::run(options, nullptr);
    std::cout.rdbuf(oldBuffer);

    assert(result.code == repaddu::core::ExitCode::success);
    const std::string output = captured.str();
    assert(output.find("REPADDU ANALYSIS REPORT") != std::string::npos);
    assert(output.find("LANGUAGE BREAKDOWN") != std::string::npos);
    assert(output.find("ANALYSIS VIEWS") != std::string::npos);
    }

void test_json_analysis_output()
    {
    repaddu::core::CliOptions options;
    options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/sample_repo";
    options.outputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/out";
    options.analyzeOnly = true;
    options.analysisEnabled = true;
    options.format = repaddu::core::OutputFormat::jsonl;

    std::ostringstream captured;
    std::streambuf* oldBuffer = std::cout.rdbuf(captured.rdbuf());
    const auto result = repaddu::cli::run(options, nullptr);
    std::cout.rdbuf(oldBuffer);

    assert(result.code == repaddu::core::ExitCode::success);
    const std::string output = captured.str();
    assert(output.find("\"type\": \"analysis_report\"") != std::string::npos);
    assert(output.find("\"views\"") != std::string::npos);
    }

int main()
    {
    test_text_analysis_output();
    test_json_analysis_output();
    std::cout << "Analysis report tests passed." << std::endl;
    return 0;
    }
