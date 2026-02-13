#include "repaddu/cli_parse.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
    {
    std::filesystem::path writeConfig(const std::string& content, const std::string& filename)
        {
        std::filesystem::path path = std::filesystem::temp_directory_path() / filename;
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
        "  \"frontmatter\": true,\n"
        "  \"emit_links\": false\n"
        "}\n";

    const auto path = writeConfig(content, "repaddu_analysis_config.json");
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
    assert(options.emitLinks == false);
    assert(options.analysisViews.size() == 2);
    assert(options.analysisViews[0] == "symbols");
    assert(options.analysisViews[1] == "dependencies");
    }

void test_analysis_yaml_config()
    {
    const std::string content =
        "analysis_enabled: true\n"
        "analysis_views: [symbols, dependencies]\n"
        "analysis_deep: true\n"
        "analysis_collapse: folder\n"
        "extract_tags: true\n"
        "tag_patterns: custom_tags.txt\n"
        "frontmatter: true\n"
        "emit_links: false\n";

    const auto path = writeConfig(content, "repaddu_analysis_config.yaml");
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
    assert(options.emitLinks == false);
    assert(options.analysisViews.size() == 2);
    assert(options.analysisViews[0] == "symbols");
    assert(options.analysisViews[1] == "dependencies");
    }

void test_analysis_yaml_config_with_inline_comments()
    {
    const std::string content =
        "# top-level comment\n"
        "analysis_enabled: yes # accepted bool synonym\n"
        "analysis_views: [\"symbols\", 'dependencies'] # list comments\n"
        "analysis_deep: no\n"
        "analysis_collapse: \"target\" # quoted scalar\n"
        "extract_tags: 1\n"
        "tag_patterns: \"custom:tags.txt\" # colon in value\n"
        "frontmatter: true\n"
        "emit_links: false\n";

    const auto path = writeConfig(content, "repaddu_analysis_config_comments.yaml");
    repaddu::core::CliOptions options;
    const auto result = repaddu::cli::loadConfigFile(path, options);
    std::filesystem::remove(path);

    assert(result.code == repaddu::core::ExitCode::success);
    assert(options.analysisEnabled == true);
    assert(options.analysisDeep == false);
    assert(options.analysisCollapse == "target");
    assert(options.extractTags == true);
    assert(options.tagPatternsPath == std::filesystem::path("custom:tags.txt"));
    assert(options.emitFrontmatter == true);
    assert(options.emitLinks == false);
    assert(options.analysisViews.size() == 2);
    assert(options.analysisViews[0] == "symbols");
    assert(options.analysisViews[1] == "dependencies");
    }

void test_analysis_yaml_keeps_hash_inside_quotes()
    {
    const std::string content =
        "analysis_enabled: true\n"
        "analysis_views: [\"symbols\", \"dependencies\"]\n"
        "analysis_collapse: none\n"
        "tag_patterns: \"custom#tags.txt\" # inline comment after quoted value\n";

    const auto path = writeConfig(content, "repaddu_analysis_config_hash.yaml");
    repaddu::core::CliOptions options;
    const auto result = repaddu::cli::loadConfigFile(path, options);
    std::filesystem::remove(path);

    assert(result.code == repaddu::core::ExitCode::success);
    assert(options.analysisEnabled == true);
    assert(options.analysisCollapse == "none");
    assert(options.tagPatternsPath == std::filesystem::path("custom#tags.txt"));
    assert(options.analysisViews.size() == 2);
    assert(options.analysisViews[0] == "symbols");
    assert(options.analysisViews[1] == "dependencies");
    }

int main()
    {
    test_analysis_config();
    test_analysis_yaml_config();
    test_analysis_yaml_config_with_inline_comments();
    test_analysis_yaml_keeps_hash_inside_quotes();
    std::cout << "Config analysis tests passed." << std::endl;
    return 0;
    }
