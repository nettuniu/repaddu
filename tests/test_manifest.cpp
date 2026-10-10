#include "repaddu/cli_parse.h"
#include "repaddu/cli_run.h"
#include "repaddu/core_types.h"
#include "repaddu/format_manifest.h"
#include "repaddu/json_lite.h"
#include "repaddu/sha256.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
    {
    std::string readText(const std::filesystem::path& path)
        {
        std::ifstream input(path);
        return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        }

    void test_manifest_builder_and_json_parse()
        {
        repaddu::format::ContextManifestData data;
        data.schemaVersion = 1;
        data.generatorName = "repaddu";
        data.generatorVersion = "0.3.0";
        data.repositoryRoot = "/workspace/project";
        data.gitCommit = "0123456789abcdef";
        data.gitBranch = "main";
        data.stats.totalFiles = 1;
        data.stats.totalBytes = 100;
        data.stats.totalTokens = 25;
        data.stats.totalOutputs = 2;

        repaddu::format::ManifestFileEntry file;
        file.path = "src/main.cpp";
        file.sha256 = repaddu::crypto::sha256("int main(){}");
        file.sizeBytes = 100;
        file.tokenCount = 25;
        file.language = ".cpp";
        file.fileClass = "source";
        file.outputChunk = "001_source.md";
        data.files.push_back(file);

        data.outputs.push_back({ "000_overview.md", 500 });
        data.outputs.push_back({ "001_source.md", 150 });

        const std::string jsonStr = repaddu::format::buildManifestJson(data);
        assert(!jsonStr.empty());

        repaddu::json::JsonValue root;
        const auto parseRes = repaddu::json::parse(jsonStr, root);
        assert(parseRes.code == repaddu::core::ExitCode::success);
        assert(root.isObject());

        const auto& obj = root.getObject();
        assert(obj.at("schema_version").getNumber() == 1.0);
        assert(obj.at("generator").getObject().at("name").getString() == "repaddu");
        assert(obj.at("generator").getObject().at("version").getString() == "0.3.0");
        assert(obj.at("repository").getObject().at("git_commit").getString() == "0123456789abcdef");
        assert(obj.at("repository").getObject().at("git_branch").getString() == "main");

        const auto& filesArray = obj.at("files").getArray();
        assert(filesArray.size() == 1);
        assert(filesArray[0].getObject().at("path").getString() == "src/main.cpp");
        assert(filesArray[0].getObject().at("sha256").getString() == repaddu::crypto::sha256("int main(){}"));
        assert(filesArray[0].getObject().at("output_chunk").getString() == "001_source.md");

        const auto& outputsArray = obj.at("outputs").getArray();
        assert(outputsArray.size() == 2);
        assert(outputsArray[0].getObject().at("filename").getString() == "000_overview.md");
        }

    void test_manifest_e2e_generation()
        {
        const std::filesystem::path outDir = std::filesystem::temp_directory_path() / "repaddu_test_manifest_e2e";
        std::error_code ec;
        std::filesystem::remove_all(outDir, ec);

        repaddu::core::CliOptions options;
        options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/sample_repo";
        options.outputPath = outDir;
        options.emitManifest = true;

        const auto result = repaddu::cli::run(options, nullptr);
        assert(result.code == repaddu::core::ExitCode::success);

        const std::filesystem::path manifestPath = outDir / "manifest.json";
        assert(std::filesystem::exists(manifestPath));

        const std::string manifestText = readText(manifestPath);
        repaddu::json::JsonValue root;
        const auto parseRes = repaddu::json::parse(manifestText, root);
        assert(parseRes.code == repaddu::core::ExitCode::success);

        const auto& obj = root.getObject();
        assert(obj.at("schema_version").getNumber() == 1.0);
        assert(obj.at("generator").getObject().at("name").getString() == "repaddu");
        assert(obj.at("stats").getObject().at("total_files").getNumber() > 0.0);

        std::filesystem::remove_all(outDir, ec);
        }

    void test_manifest_disabled_flag()
        {
        const std::filesystem::path outDir = std::filesystem::temp_directory_path() / "repaddu_test_manifest_disabled";
        std::error_code ec;
        std::filesystem::remove_all(outDir, ec);

        repaddu::core::CliOptions options;
        options.inputPath = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures/sample_repo";
        options.outputPath = outDir;
        options.emitManifest = false;

        const auto result = repaddu::cli::run(options, nullptr);
        assert(result.code == repaddu::core::ExitCode::success);

        const std::filesystem::path manifestPath = outDir / "manifest.json";
        assert(!std::filesystem::exists(manifestPath));

        std::filesystem::remove_all(outDir, ec);
        }

    void test_cli_parsing_manifest_flags()
        {
        {
            const std::vector<std::string> args = { "repaddu", "-i", ".", "-o", "out", "--no-manifest" };
            const auto parsed = repaddu::cli::parseArgs(args);
            assert(parsed.result.code == repaddu::core::ExitCode::success);
            assert(parsed.options.emitManifest == false);
        }
        {
            const std::vector<std::string> args = { "repaddu", "-i", ".", "-o", "out", "--emit-manifest" };
            const auto parsed = repaddu::cli::parseArgs(args);
            assert(parsed.result.code == repaddu::core::ExitCode::success);
            assert(parsed.options.emitManifest == true);
        }
        }
    }

int main()
    {
    test_manifest_builder_and_json_parse();
    test_manifest_e2e_generation();
    test_manifest_disabled_flag();
    test_cli_parsing_manifest_flags();
    std::cout << "All manifest tests passed successfully.\n";
    return 0;
    }
