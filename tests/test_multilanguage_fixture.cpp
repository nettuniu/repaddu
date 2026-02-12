#include "repaddu/core_types.h"
#include "repaddu/grouping_strategies.h"
#include "repaddu/io_traversal.h"
#include "repaddu/language_profiles.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>

namespace
    {
    repaddu::io::TraversalResult traverseFixture(const std::filesystem::path& path)
        {
        repaddu::core::CliOptions options;
        options.inputPath = path;
        options.outputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "out";

        repaddu::io::TraversalResult traversal;
        const auto result = repaddu::io::traverseRepository(options, traversal);
        assert(result.code == repaddu::core::ExitCode::success);
        return traversal;
        }

    bool containsPath(const std::vector<std::filesystem::path>& paths, const std::filesystem::path& needle)
        {
        for (const auto& path : paths)
            {
            if (path == needle)
                {
                return true;
                }
            }
        return false;
        }
    }

void test_auto_detection_per_language()
    {
    const std::filesystem::path root = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "multi_language";

    {
    const auto traversal = traverseFixture(root / "c");
    const auto detected = repaddu::core::detectLanguageAndBuildSystem(traversal.files);
    assert(detected.languageId == "c");
    assert(detected.buildSystemId == "make");
    }

    {
    const auto traversal = traverseFixture(root / "cpp");
    const auto detected = repaddu::core::detectLanguageAndBuildSystem(traversal.files);
    assert(detected.languageId == "cpp");
    assert(detected.buildSystemId == "cmake");
    }

    {
    const auto traversal = traverseFixture(root / "rust");
    const auto detected = repaddu::core::detectLanguageAndBuildSystem(traversal.files);
    assert(detected.languageId == "rust");
    assert(detected.buildSystemId == "cargo");
    }

    {
    const auto traversal = traverseFixture(root / "python");
    const auto detected = repaddu::core::detectLanguageAndBuildSystem(traversal.files);
    assert(detected.languageId == "python");
    assert(detected.buildSystemId == "python");
    }
    }

void test_extension_filtering()
    {
    const std::filesystem::path root = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "multi_language";
    const auto traversal = traverseFixture(root);

    repaddu::core::CliOptions options;
    options.extensions = { "rs" };
    options.includeHeaders = false;
    options.includeSources = false;

    repaddu::core::RunResult groupingResult;
    const auto grouped = repaddu::grouping::filterAndGroupFiles(options, traversal.files, nullptr, groupingResult);
    assert(groupingResult.code == repaddu::core::ExitCode::success);
    assert(grouped.includedIndices.size() == 1);

    const auto& file = traversal.files[grouped.includedIndices[0]];
    assert(file.relativePath == std::filesystem::path("rust/src/main.rs"));
    }

void test_build_file_aggregation()
    {
    const std::filesystem::path root = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "multi_language";
    const auto traversal = traverseFixture(root);

    assert(containsPath(traversal.buildFiles, std::filesystem::path("c/Makefile")));
    assert(containsPath(traversal.buildFiles, std::filesystem::path("cpp/CMakeLists.txt")));
    assert(containsPath(traversal.buildFiles, std::filesystem::path("rust/Cargo.toml")));
    assert(containsPath(traversal.buildFiles, std::filesystem::path("python/pyproject.toml")));
    }

int main()
    {
    test_auto_detection_per_language();
    test_extension_filtering();
    test_build_file_aggregation();
    std::cout << "Multi-language fixture tests passed." << std::endl;
    return 0;
    }
