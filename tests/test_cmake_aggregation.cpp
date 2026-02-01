#include "repaddu/core_types.h"
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

    expectEqual(traversal.cmakeLists.size(), 2, "CMakeLists aggregation should find two files");
    if (traversal.cmakeLists.size() == 2)
        {
        const std::string first = traversal.cmakeLists[0].generic_string();
        const std::string second = traversal.cmakeLists[1].generic_string();
        expectTrue(first < second, "CMakeLists paths should be sorted");
        }

    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
