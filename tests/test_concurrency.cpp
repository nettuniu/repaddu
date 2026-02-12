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

    bool sameFileEntry(const repaddu::core::FileEntry& lhs, const repaddu::core::FileEntry& rhs)
        {
        return lhs.relativePath == rhs.relativePath
            && lhs.sizeBytes == rhs.sizeBytes
            && lhs.extensionLower == rhs.extensionLower
            && lhs.isBinary == rhs.isBinary
            && lhs.fileClass == rhs.fileClass;
        }

    void compareTraversal(const repaddu::io::TraversalResult& singleThread,
        const repaddu::io::TraversalResult& parallelThread)
        {
        expectTrue(singleThread.files.size() == parallelThread.files.size(),
            "File count must match between single-thread and parallel traversal");
        expectTrue(singleThread.directories.size() == parallelThread.directories.size(),
            "Directory count must match between single-thread and parallel traversal");
        expectTrue(singleThread.cmakeLists.size() == parallelThread.cmakeLists.size(),
            "CMake list count must match between single-thread and parallel traversal");
        expectTrue(singleThread.buildFiles.size() == parallelThread.buildFiles.size(),
            "Build file count must match between single-thread and parallel traversal");

        if (singleThread.files.size() == parallelThread.files.size())
            {
            for (std::size_t index = 0; index < singleThread.files.size(); ++index)
                {
                expectTrue(sameFileEntry(singleThread.files[index], parallelThread.files[index]),
                    "File entry mismatch between single-thread and parallel traversal");
                }
            }

        if (singleThread.directories.size() == parallelThread.directories.size())
            {
            for (std::size_t index = 0; index < singleThread.directories.size(); ++index)
                {
                expectTrue(singleThread.directories[index] == parallelThread.directories[index],
                    "Directory ordering mismatch between single-thread and parallel traversal");
                }
            }

        if (singleThread.cmakeLists.size() == parallelThread.cmakeLists.size())
            {
            for (std::size_t index = 0; index < singleThread.cmakeLists.size(); ++index)
                {
                expectTrue(singleThread.cmakeLists[index] == parallelThread.cmakeLists[index],
                    "CMake path mismatch between single-thread and parallel traversal");
                }
            }

        if (singleThread.buildFiles.size() == parallelThread.buildFiles.size())
            {
            for (std::size_t index = 0; index < singleThread.buildFiles.size(); ++index)
                {
                expectTrue(singleThread.buildFiles[index] == parallelThread.buildFiles[index],
                    "Build file path mismatch between single-thread and parallel traversal");
                }
            }
        }

    void testTraversalDeterminism(const std::filesystem::path& repoRoot)
        {
        repaddu::core::CliOptions singleOptions;
        singleOptions.inputPath = repoRoot;
        singleOptions.outputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "out";
        singleOptions.includeHidden = true;
        singleOptions.parallelTraversal = false;

        repaddu::core::CliOptions parallelOptions = singleOptions;
        parallelOptions.parallelTraversal = true;

        repaddu::io::TraversalResult singleTraversal;
        repaddu::io::TraversalResult parallelTraversal;

        repaddu::core::RunResult singleResult = repaddu::io::traverseRepository(singleOptions, singleTraversal);
        repaddu::core::RunResult parallelResult = repaddu::io::traverseRepository(parallelOptions, parallelTraversal);

        expectTrue(singleResult.code == repaddu::core::ExitCode::success,
            "Single-thread traversal must succeed");
        expectTrue(parallelResult.code == repaddu::core::ExitCode::success,
            "Parallel traversal must succeed");

        if (singleResult.code == repaddu::core::ExitCode::success
            && parallelResult.code == repaddu::core::ExitCode::success)
            {
            compareTraversal(singleTraversal, parallelTraversal);
            }
        }
    }

int main()
    {
    const std::filesystem::path fixtureRoot = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures";
    testTraversalDeterminism(fixtureRoot / "sample_repo");
    testTraversalDeterminism(fixtureRoot / "multi_language");

    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
