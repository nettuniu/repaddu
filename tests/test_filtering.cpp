#include "repaddu/core_types.h"
#include "repaddu/grouping_strategies.h"
#include "repaddu/io_traversal.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace
    {
    int g_failures = 0;

    void expectTrue(bool value, const char* message)
        {
        if (!value)
            {
            std::cerr << "FAIL: " << message << "\n";
            ++g_failures;
            }
        }

    void expectEqual(std::size_t lhs, std::size_t rhs, const char* message)
        {
        if (lhs != rhs)
            {
            std::cerr << "FAIL: " << message << " (" << lhs << " vs " << rhs << ")\n";
            ++g_failures;
            }
        }
    }

int main()
    {
    repaddu::core::CliOptions options;
    options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/sample_repo";
    options.outputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/out";

    repaddu::io::TraversalResult traversal;
    repaddu::core::RunResult traversalResult = repaddu::io::traverseRepository(options, traversal);
    expectTrue(traversalResult.code == repaddu::core::ExitCode::success, "Traversal should succeed");

    bool hasGit = false;
    bool hasHidden = false;
    for (const auto& entry : traversal.files)
        {
        const std::string rel = entry.relativePath.generic_string();
        if (rel.find(".git") != std::string::npos)
            {
            hasGit = true;
            }
        if (!rel.empty() && rel.front() == '.')
            {
            hasHidden = true;
            }
        if (rel.find("/.hidden/") != std::string::npos)
            {
            hasHidden = true;
            }
        }
    expectTrue(!hasGit, "Traversal should exclude .git paths");
    expectTrue(!hasHidden, "Traversal should exclude hidden paths by default");

    repaddu::core::RunResult groupingResult;
    options.includeHeaders = true;
    options.includeSources = false;
    repaddu::grouping::GroupingResult grouped = repaddu::grouping::filterAndGroupFiles(options, traversal.files, nullptr, groupingResult);
    expectTrue(groupingResult.code == repaddu::core::ExitCode::success, "Filtering should succeed");
    expectEqual(grouped.includedIndices.size(), 1, "Headers-only filtering should include one file");

    options.extensions = { "bin" };
    options.includeHeaders = false;
    options.includeSources = false;
    options.includeBinaries = false;
    grouped = repaddu::grouping::filterAndGroupFiles(options, traversal.files, nullptr, groupingResult);
    expectEqual(grouped.includedIndices.size(), 0, "Binary exclusion should drop binary file");

    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
