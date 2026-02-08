#include "repaddu/core_types.h"
#include "repaddu/io_traversal.h"
#include "repaddu/format_language_report.h"

#include <cassert>
#include <filesystem>
#include <iostream>

int main()
    {
    repaddu::core::CliOptions options;
    options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/picobench";
    options.outputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/out";
    options.includeHidden = true;

    repaddu::io::TraversalResult traversal;
    const auto result = repaddu::io::traverseRepository(options, traversal);
    assert(result.code == repaddu::core::ExitCode::success);

    bool sawHeader = false;
    bool sawExample = false;
    for (const auto& entry : traversal.files)
        {
        if (entry.relativePath == std::filesystem::path("include/picobench/picobench.hpp"))
            {
            sawHeader = true;
            }
        if (entry.relativePath == std::filesystem::path("example/basic.cpp"))
            {
            sawExample = true;
            }
        }

    assert(sawHeader);
    assert(sawExample);

    const std::string report = repaddu::format::renderLanguageReport(options, traversal.files);
    assert(report.find("C++") != std::string::npos);

    std::cout << "Picobench fixture tests passed." << std::endl;
    return 0;
    }
