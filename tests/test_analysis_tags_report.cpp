#include "repaddu/cli_run.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace
    {
    struct TempFixture
        {
        std::filesystem::path root;
        std::filesystem::path sourceFile;
        std::filesystem::path patternsFile;
        std::filesystem::path outputDir;
        };

    TempFixture createFixture()
        {
        TempFixture fixture;
        fixture.root = std::filesystem::temp_directory_path() / "repaddu_tags_report_fixture";
        std::error_code errorCode;
        std::filesystem::remove_all(fixture.root, errorCode);
        std::filesystem::create_directories(fixture.root / "src");
        std::filesystem::create_directories(fixture.root / "out");

        fixture.sourceFile = fixture.root / "src" / "main.cpp";
        fixture.patternsFile = fixture.root / "tags.txt";
        fixture.outputDir = fixture.root / "out";

        {
        std::ofstream source(fixture.sourceFile);
        source << "// TODO: implement parser\\n";
        source << "// NOTE: custom marker\\n";
        source << "int main() { return 0; }\\n";
        }

        {
        std::ofstream patterns(fixture.patternsFile);
        patterns << "NOTE\\n";
        }

        return fixture;
        }
    }

void test_tag_summary_in_analyze_only_output()
    {
    const TempFixture fixture = createFixture();

    repaddu::core::CliOptions options;
    options.inputPath = fixture.root;
    options.outputPath = fixture.outputDir;
    options.analyzeOnly = true;
    options.extractTags = true;
    options.tagPatternsPath = fixture.patternsFile;

    std::ostringstream captured;
    std::streambuf* oldBuffer = std::cout.rdbuf(captured.rdbuf());

    const repaddu::core::RunResult result = repaddu::cli::run(options, nullptr);

    std::cout.rdbuf(oldBuffer);

    assert(result.code == repaddu::core::ExitCode::success);

    const std::string output = captured.str();
    assert(output.find("TAG SUMMARY") != std::string::npos);
    assert(output.find("TODO") != std::string::npos);
    assert(output.find("NOTE") != std::string::npos);
    assert(output.find("src/main.cpp") != std::string::npos);

    std::error_code errorCode;
    std::filesystem::remove_all(fixture.root, errorCode);
    }

int main()
    {
    test_tag_summary_in_analyze_only_output();
    std::cout << "Analysis tag report tests passed." << std::endl;
    return 0;
    }
