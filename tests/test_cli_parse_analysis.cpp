#include "repaddu/cli_parse.h"

#include <cassert>
#include <filesystem>
#include <iostream>

void test_analysis_flags()
    {
    std::vector<std::string> args =
        {
        "repaddu",
        "--analysis",
        "--analysis-views",
        "symbols,dependencies",
        "--analysis-deep",
        "--analysis-collapse",
        "folder",
        "--extract-tags",
        "--tag-patterns",
        "tags.txt",
        "--frontmatter",
        "--no-links",
        "-i",
        "input",
        "-o",
        "out"
        };

    const auto result = repaddu::cli::parseArgs(args);
    assert(result.result.code == repaddu::core::ExitCode::success);
    assert(result.options.analysisEnabled == true);
    assert(result.options.analysisDeep == true);
    assert(result.options.analysisCollapse == "folder");
    assert(result.options.extractTags == true);
    assert(result.options.tagPatternsPath == std::filesystem::path("tags.txt"));
    assert(result.options.emitFrontmatter == true);
    assert(result.options.emitLinks == false);
    assert(result.options.analysisViews.size() == 2);
    assert(result.options.analysisViews[0] == "symbols");
    assert(result.options.analysisViews[1] == "dependencies");
    }

void test_invalid_collapse()
    {
    std::vector<std::string> args =
        {
        "repaddu",
        "--analysis-collapse",
        "weird",
        "-i",
        "input",
        "-o",
        "out"
        };

    const auto result = repaddu::cli::parseArgs(args);
    assert(result.result.code == repaddu::core::ExitCode::invalid_usage);
    }

void test_parallel_flags()
    {
    std::vector<std::string> args =
        {
        "repaddu",
        "--single-thread",
        "-i",
        "input",
        "-o",
        "out"
        };

    auto result = repaddu::cli::parseArgs(args);
    assert(result.result.code == repaddu::core::ExitCode::success);
    assert(result.options.parallelTraversal == false);

    args =
        {
        "repaddu",
        "--single-thread",
        "--parallel-traversal",
        "-i",
        "input",
        "-o",
        "out"
        };
    result = repaddu::cli::parseArgs(args);
    assert(result.result.code == repaddu::core::ExitCode::success);
    assert(result.options.parallelTraversal == true);
    }

int main()
    {
    test_analysis_flags();
    test_invalid_collapse();
    test_parallel_flags();
    std::cout << "CLI analysis parse tests passed." << std::endl;
    return 0;
    }
