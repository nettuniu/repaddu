#include "repaddu/core_types.h"
#include "repaddu/grouping_strategies.h"
#include "repaddu/io_traversal.h"

#include <cassert>
#include <filesystem>
#include <iostream>

namespace
    {
    bool hasPath(const std::vector<std::size_t>& indices,
        const std::vector<repaddu::core::FileEntry>& files,
        const std::filesystem::path& target)
        {
        for (std::size_t index : indices)
            {
            if (files[index].relativePath == target)
                {
                return true;
                }
            }
        return false;
        }
    }

void test_isolate_docs_with_directory_grouping()
    {
    repaddu::core::CliOptions options;
    options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/sample_repo";
    options.outputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/out";
    options.groupBy = repaddu::core::GroupingMode::directory;
    options.isolateDocs = true;

    repaddu::io::TraversalResult traversal;
    const auto traversalResult = repaddu::io::traverseRepository(options, traversal);
    assert(traversalResult.code == repaddu::core::ExitCode::success);

    repaddu::core::RunResult groupingResult;
    const auto grouped = repaddu::grouping::filterAndGroupFiles(options, traversal.files, nullptr, groupingResult);
    assert(groupingResult.code == repaddu::core::ExitCode::success);

    bool hasDocsGroup = false;
    for (const auto& group : grouped.groups)
        {
        if (group.name == "documentation")
            {
            hasDocsGroup = true;
            assert(hasPath(group.fileIndices, traversal.files, std::filesystem::path("docs/readme.txt")));
            assert(!hasPath(group.fileIndices, traversal.files, std::filesystem::path("CMakeLists.txt")));
            assert(!hasPath(group.fileIndices, traversal.files, std::filesystem::path("subproj/CMakeLists.txt")));
            }
        }
    assert(hasDocsGroup);
    }

void test_isolate_docs_with_size_grouping()
    {
    repaddu::core::CliOptions options;
    options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/sample_repo";
    options.outputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/out";
    options.groupBy = repaddu::core::GroupingMode::size;
    options.isolateDocs = true;

    repaddu::io::TraversalResult traversal;
    const auto traversalResult = repaddu::io::traverseRepository(options, traversal);
    assert(traversalResult.code == repaddu::core::ExitCode::success);

    repaddu::core::RunResult groupingResult;
    const auto grouped = repaddu::grouping::filterAndGroupFiles(options, traversal.files, nullptr, groupingResult);
    assert(groupingResult.code == repaddu::core::ExitCode::success);

    bool hasDocsGroup = false;
    bool hasSizeGroup = false;
    for (const auto& group : grouped.groups)
        {
        if (group.name == "documentation")
            {
            hasDocsGroup = true;
            assert(hasPath(group.fileIndices, traversal.files, std::filesystem::path("docs/readme.txt")));
            }
        if (group.name == "size")
            {
            hasSizeGroup = true;
            assert(!hasPath(group.fileIndices, traversal.files, std::filesystem::path("docs/readme.txt")));
            }
        }

    assert(hasDocsGroup);
    assert(hasSizeGroup);

    repaddu::core::RunResult chunkResult;
    const auto chunks = repaddu::grouping::chunkGroups(options, traversal.files, grouped.groups, chunkResult);
    assert(chunkResult.code == repaddu::core::ExitCode::success);

    bool hasDocsChunk = false;
    bool hasSizeChunk = false;
    for (const auto& chunk : chunks)
        {
        if (chunk.category == "documentation")
            {
            hasDocsChunk = true;
            assert(hasPath(chunk.fileIndices, traversal.files, std::filesystem::path("docs/readme.txt")));
            }
        if (chunk.category == "size")
            {
            hasSizeChunk = true;
            }
        }
    assert(hasDocsChunk);
    assert(hasSizeChunk);
    }

int main()
    {
    test_isolate_docs_with_directory_grouping();
    test_isolate_docs_with_size_grouping();
    std::cout << "Isolate docs tests passed." << std::endl;
    return 0;
    }
