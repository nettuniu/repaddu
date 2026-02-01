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

    repaddu::core::RunResult groupingResult;
    options.includeHeaders = true;
    options.includeSources = true;
    options.groupBy = repaddu::core::GroupingMode::directory;
    repaddu::grouping::GroupingResult grouped = repaddu::grouping::filterAndGroupFiles(options, traversal.files, nullptr, groupingResult);
    expectEqual(grouped.groups.size(), 2, "Directory grouping should produce include/src groups");

    options.groupBy = repaddu::core::GroupingMode::type;
    grouped = repaddu::grouping::filterAndGroupFiles(options, traversal.files, nullptr, groupingResult);
    expectEqual(grouped.groups.size(), 2, "Type grouping should produce header/source groups");

    options.groupBy = repaddu::core::GroupingMode::size;
    options.maxFiles = 2;
    grouped = repaddu::grouping::filterAndGroupFiles(options, traversal.files, nullptr, groupingResult);
    repaddu::core::RunResult chunkResult;
    std::vector<repaddu::core::OutputChunk> chunks = repaddu::grouping::chunkGroups(options, traversal.files, grouped.groups, chunkResult);
    expectTrue(chunkResult.code == repaddu::core::ExitCode::success, "Chunking should succeed");
    expectEqual(chunks.size(), 2, "Size grouping should create two chunks when max-files=2");

    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
