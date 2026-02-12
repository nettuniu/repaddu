#include "repaddu/analysis_tags.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

void test_tag_extraction()
    {
    repaddu::analysis::TagExtractor extractor;
    
    std::string content = 
        "// TODO: implement this\n"
        "int x = 1;\n"
        "/* FIXME fix the bug here */\n"
        "// BUG: it crashes\n"
        "// HACK: don't look here\n"
        "// Normal comment\n";

    auto matches = extractor.extract(content, "test.cpp");
    
    assert(matches.size() == 4);
    
    assert(matches[0].tag == "TODO");
    assert(matches[0].content == "implement this");
    assert(matches[0].lineNumber == 1);

    assert(matches[1].tag == "FIXME");
    assert(matches[1].content == "fix the bug here */"); // include the */ for now as we don't parse comments fully
    assert(matches[1].lineNumber == 3);

    assert(matches[2].tag == "BUG");
    assert(matches[2].content == "it crashes");
    assert(matches[2].lineNumber == 4);

    assert(matches[3].tag == "HACK");
    assert(matches[3].content == "don't look here");
    assert(matches[3].lineNumber == 5);

    std::cout << "Tag extraction tests passed." << std::endl;
    }

void test_tag_patterns_from_file()
    {
    const std::filesystem::path tempFile = std::filesystem::temp_directory_path()
        / "repaddu_tag_patterns.txt";

    {
    std::ofstream output(tempFile);
    output << "# custom tags\n";
    output << "NOTE\n";
    output << "TECHDEBT\n";
    output << "NOTE\n";
    }

    repaddu::analysis::TagExtractor extractor;
    const bool loaded = extractor.loadTagPatternsFromFile(tempFile, true);
    assert(loaded);

    const std::string content =
        "// TODO: should be ignored after replacement\n"
        "// NOTE: keep this note\n"
        "// TECHDEBT reduce complexity\n";

    const auto matches = extractor.extract(content, "custom.cpp");
    assert(matches.size() == 2);
    assert(matches[0].tag == "NOTE");
    assert(matches[1].tag == "TECHDEBT");

    std::error_code errorCode;
    std::filesystem::remove(tempFile, errorCode);
    }

int main()
    {
    test_tag_extraction();
    test_tag_patterns_from_file();
    return 0;
    }
