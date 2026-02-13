#include "repaddu/cli_parse.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
    {
    std::filesystem::path writeConfig(const std::string& content)
        {
        std::filesystem::path path = std::filesystem::temp_directory_path() / "repaddu_analysis_config.json";
        std::ofstream ofs(path);
        ofs << content;
        return path;
        }
    }

void test_analysis_config()
    {
    const std::string content =
        "{\n"
        "  \"analysis_enabled\": true,\n"
        "  \"analysis_views\": [\"symbols\", \"dependencies\"],\n"
        "  \"analysis_deep\": true,\n"
        "  \"analysis_collapse\": \"folder\",\n"
        "  \"extract_tags\": true,\n"
        "  \"tag_patterns\": \"custom_tags.txt\",\n"
        "  \"frontmatter\": true\n"
        "}\n";

    const auto path = writeConfig(content);
    repaddu::core::CliOptions options;
    const auto result = repaddu::cli::loadConfigFile(path, options);
    std::filesystem::remove(path);

    assert(result.code == repaddu::core::ExitCode::success);
    assert(options.analysisEnabled == true);
    assert(options.analysisDeep == true);
    assert(options.analysisCollapse == "folder");
    assert(options.extractTags == true);
    assert(options.tagPatternsPath == std::filesystem::path("custom_tags.txt"));
    assert(options.emitFrontmatter == true);
    assert(options.analysisViews.size() == 2);
    assert(options.analysisViews[0] == "symbols");
    assert(options.analysisViews[1] == "dependencies");
    }

int main()
    {
    test_analysis_config();
    std::cout << "Config analysis tests passed." << std::endl;
    return 0;
    }
