#include "repaddu/cli_parse.h"
#include "repaddu/cli_run.h"

#include <cassert>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

void test_config_path_parsing()
    {
    const std::vector<std::string> args =
        {
            "repaddu",
            "--config",
            "custom.json",
            "--init"
        };

    const auto result = repaddu::cli::parseArgs(args);
    assert(result.result.code == repaddu::core::ExitCode::success);
    assert(result.options.generateConfig == true);
    assert(result.options.configPath == std::filesystem::path("custom.json"));
    }

void test_config_missing_value()
    {
    const std::vector<std::string> args =
        {
            "repaddu",
            "--config"
        };

    const auto result = repaddu::cli::parseArgs(args);
    assert(result.result.code == repaddu::core::ExitCode::invalid_usage);
    }

void test_generate_config_custom_path()
    {
    const std::filesystem::path tempPath = std::filesystem::temp_directory_path()
        / "repaddu_custom_config_test.json";
    std::error_code errorCode;
    std::filesystem::remove(tempPath, errorCode);

    repaddu::core::CliOptions options;
    options.generateConfig = true;
    options.configPath = tempPath;

    const auto runResult = repaddu::cli::run(options, nullptr);
    assert(runResult.code == repaddu::core::ExitCode::success);
    assert(std::filesystem::exists(tempPath));

    std::ifstream ifs(tempPath);
    assert(static_cast<bool>(ifs));
    std::ostringstream content;
    content << ifs.rdbuf();
    const std::string value = content.str();

    assert(value.find("{\n") != std::string::npos);
    assert(value.find("\"input\": \".\"") != std::string::npos);
    assert(value.find("input: .\n") == std::string::npos);

    std::filesystem::remove(tempPath, errorCode);
    }

void test_generate_config_custom_yaml_path()
    {
    const std::filesystem::path tempPath = std::filesystem::temp_directory_path()
        / "repaddu_custom_config_test.yaml";
    std::error_code errorCode;
    std::filesystem::remove(tempPath, errorCode);

    repaddu::core::CliOptions options;
    options.generateConfig = true;
    options.configPath = tempPath;

    const auto runResult = repaddu::cli::run(options, nullptr);
    assert(runResult.code == repaddu::core::ExitCode::success);
    assert(std::filesystem::exists(tempPath));

    std::ifstream ifs(tempPath);
    assert(static_cast<bool>(ifs));
    std::ostringstream content;
    content << ifs.rdbuf();
    const std::string value = content.str();

    assert(value.find("input: .\n") != std::string::npos);
    assert(value.find("format: markdown\n") != std::string::npos);
    assert(value.find("\"input\"") == std::string::npos);

    std::filesystem::remove(tempPath, errorCode);
    }

void test_generate_config_custom_yml_uppercase_path()
    {
    const std::filesystem::path tempPath = std::filesystem::temp_directory_path()
        / "repaddu_custom_config_test.YML";
    std::error_code errorCode;
    std::filesystem::remove(tempPath, errorCode);

    repaddu::core::CliOptions options;
    options.generateConfig = true;
    options.configPath = tempPath;

    const auto runResult = repaddu::cli::run(options, nullptr);
    assert(runResult.code == repaddu::core::ExitCode::success);
    assert(std::filesystem::exists(tempPath));

    std::ifstream ifs(tempPath);
    assert(static_cast<bool>(ifs));
    std::ostringstream content;
    content << ifs.rdbuf();
    const std::string value = content.str();

    assert(value.find("group_by: directory\n") != std::string::npos);
    assert(value.find("\"group_by\"") == std::string::npos);

    std::filesystem::remove(tempPath, errorCode);
    }

void test_generated_json_and_yaml_defaults_are_equivalent()
    {
    const std::filesystem::path jsonPath = std::filesystem::temp_directory_path()
        / "repaddu_generated_defaults_equivalence.json";
    const std::filesystem::path yamlPath = std::filesystem::temp_directory_path()
        / "repaddu_generated_defaults_equivalence.yaml";
    std::error_code errorCode;
    std::filesystem::remove(jsonPath, errorCode);
    std::filesystem::remove(yamlPath, errorCode);

    repaddu::core::CliOptions generateOptions;
    generateOptions.generateConfig = true;

    generateOptions.configPath = jsonPath;
    assert(repaddu::cli::run(generateOptions, nullptr).code == repaddu::core::ExitCode::success);

    generateOptions.configPath = yamlPath;
    assert(repaddu::cli::run(generateOptions, nullptr).code == repaddu::core::ExitCode::success);

    repaddu::core::CliOptions fromJson;
    repaddu::core::CliOptions fromYaml;
    assert(repaddu::cli::loadConfigFile(jsonPath, fromJson).code == repaddu::core::ExitCode::success);
    assert(repaddu::cli::loadConfigFile(yamlPath, fromYaml).code == repaddu::core::ExitCode::success);

    assert(fromJson.inputPath == fromYaml.inputPath);
    assert(fromJson.outputPath == fromYaml.outputPath);
    assert(fromJson.maxFiles == fromYaml.maxFiles);
    assert(fromJson.maxBytes == fromYaml.maxBytes);
    assert(fromJson.numberWidth == fromYaml.numberWidth);
    assert(fromJson.includeHeaders == fromYaml.includeHeaders);
    assert(fromJson.includeSources == fromYaml.includeSources);
    assert(fromJson.includeHidden == fromYaml.includeHidden);
    assert(fromJson.includeBinaries == fromYaml.includeBinaries);
    assert(fromJson.followSymlinks == fromYaml.followSymlinks);
    assert(fromJson.headersFirst == fromYaml.headersFirst);
    assert(fromJson.emitTree == fromYaml.emitTree);
    assert(fromJson.emitCMake == fromYaml.emitCMake);
    assert(fromJson.emitBuildFiles == fromYaml.emitBuildFiles);
    assert(fromJson.emitLinks == fromYaml.emitLinks);
    assert(fromJson.emitFrontmatter == fromYaml.emitFrontmatter);
    assert(fromJson.maxFileSize == fromYaml.maxFileSize);
    assert(fromJson.forceLargeFiles == fromYaml.forceLargeFiles);
    assert(fromJson.redactPii == fromYaml.redactPii);
    assert(fromJson.analyzeOnly == fromYaml.analyzeOnly);
    assert(fromJson.analysisEnabled == fromYaml.analysisEnabled);
    assert(fromJson.analysisViews == fromYaml.analysisViews);
    assert(fromJson.analysisDeep == fromYaml.analysisDeep);
    assert(fromJson.analysisCollapse == fromYaml.analysisCollapse);
    assert(fromJson.extractTags == fromYaml.extractTags);
    assert(fromJson.tagPatternsPath == fromYaml.tagPatternsPath);
    assert(fromJson.isolateDocs == fromYaml.isolateDocs);
    assert(fromJson.dryRun == fromYaml.dryRun);
    assert(fromJson.parallelTraversal == fromYaml.parallelTraversal);
    assert(fromJson.format == fromYaml.format);
    assert(fromJson.groupBy == fromYaml.groupBy);
    assert(fromJson.markers == fromYaml.markers);
    assert(fromJson.extensions == fromYaml.extensions);
    assert(fromJson.excludeExtensions == fromYaml.excludeExtensions);

    std::filesystem::remove(jsonPath, errorCode);
    std::filesystem::remove(yamlPath, errorCode);
    }

int main()
    {
    test_config_path_parsing();
    test_config_missing_value();
    test_generate_config_custom_path();
    test_generate_config_custom_yaml_path();
    test_generate_config_custom_yml_uppercase_path();
    test_generated_json_and_yaml_defaults_are_equivalent();
    std::cout << "CLI config path tests passed." << std::endl;
    return 0;
    }
