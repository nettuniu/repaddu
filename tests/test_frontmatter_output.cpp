#include "repaddu/format_writer.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
    {
    std::string readText(const std::filesystem::path& path)
        {
        std::ifstream input(path);
        std::string data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        return data;
        }

    repaddu::core::FileEntry makeEntry(const std::filesystem::path& repoRoot)
        {
        repaddu::core::FileEntry entry;
        entry.absolutePath = repoRoot / "src" / "main.cpp";
        entry.relativePath = std::filesystem::path("src/main.cpp");
        entry.extensionLower = ".cpp";
        entry.fileClass = repaddu::core::FileClass::source;
        entry.sizeBytes = std::filesystem::file_size(entry.absolutePath);
        return entry;
        }

    std::filesystem::path makeTempOutDir(const std::string& name)
        {
        const std::filesystem::path path = std::filesystem::temp_directory_path() / name;
        std::error_code errorCode;
        std::filesystem::remove_all(path, errorCode);
        std::filesystem::create_directories(path);
        return path;
        }

    repaddu::core::FileEntry makeCustomEntry(const std::filesystem::path& absolutePath, const std::filesystem::path& relativePath)
        {
        repaddu::core::FileEntry entry;
        entry.absolutePath = absolutePath;
        entry.relativePath = relativePath;
        entry.extensionLower = repaddu::core::toLowerCopy(absolutePath.extension().string());
        entry.fileClass = repaddu::core::classifyExtension(entry.extensionLower);
        entry.sizeBytes = std::filesystem::file_size(absolutePath);
        return entry;
        }
    }

void test_frontmatter_enabled()
    {
    const std::filesystem::path repoRoot = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "sample_repo";

    repaddu::core::CliOptions options;
    options.inputPath = repoRoot;
    options.outputPath = makeTempOutDir("repaddu_frontmatter_on");
    options.emitTree = false;
    options.emitCMake = false;
    options.emitBuildFiles = false;
    options.emitFrontmatter = true;

    std::vector<repaddu::core::FileEntry> files = { makeEntry(repoRoot) };
    repaddu::core::OutputChunk chunk;
    chunk.category = "source";
    chunk.title = "source";
    chunk.fileIndices = { 0 };

    const auto result = repaddu::format::writeOutputs(options, files, { chunk }, "", {}, {});
    assert(result.code == repaddu::core::ExitCode::success);

    const std::string content = readText(options.outputPath / "001_source.md");
    assert(content.find("---\npath: src/main.cpp\n") != std::string::npos);
    }

void test_frontmatter_disabled()
    {
    const std::filesystem::path repoRoot = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "sample_repo";

    repaddu::core::CliOptions options;
    options.inputPath = repoRoot;
    options.outputPath = makeTempOutDir("repaddu_frontmatter_off");
    options.emitTree = false;
    options.emitCMake = false;
    options.emitBuildFiles = false;
    options.emitFrontmatter = false;

    std::vector<repaddu::core::FileEntry> files = { makeEntry(repoRoot) };
    repaddu::core::OutputChunk chunk;
    chunk.category = "source";
    chunk.title = "source";
    chunk.fileIndices = { 0 };

    const auto result = repaddu::format::writeOutputs(options, files, { chunk }, "", {}, {});
    assert(result.code == repaddu::core::ExitCode::success);

    const std::string content = readText(options.outputPath / "001_source.md");
    assert(content.find("---\npath: src/main.cpp\n") == std::string::npos);
    }

void test_overview_links_disabled()
    {
    const std::filesystem::path repoRoot = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "sample_repo";

    repaddu::core::CliOptions options;
    options.inputPath = repoRoot;
    options.outputPath = makeTempOutDir("repaddu_overview_no_links");
    options.emitTree = false;
    options.emitCMake = false;
    options.emitBuildFiles = false;
    options.emitLinks = false;

    std::vector<repaddu::core::FileEntry> files = { makeEntry(repoRoot) };
    repaddu::core::OutputChunk chunk;
    chunk.category = "source";
    chunk.title = "source";
    chunk.fileIndices = { 0 };

    const auto result = repaddu::format::writeOutputs(options, files, { chunk }, "", {}, {});
    assert(result.code == repaddu::core::ExitCode::success);

    const std::string content = readText(options.outputPath / "000_overview.md");
    assert(content.find("[001_source.md](001_source.md)") == std::string::npos);
    assert(content.find("- 001_source.md") != std::string::npos);
    }

void test_jsonl_output_writes_dataset_and_escapes_content()
    {
    const std::filesystem::path repoRoot = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "sample_repo";
    const std::filesystem::path inputRoot = makeTempOutDir("repaddu_jsonl_input");
    const std::filesystem::path outputRoot = makeTempOutDir("repaddu_jsonl_output");
    const std::filesystem::path sourcePath = inputRoot / "src" / "escaped.cpp";
    std::filesystem::create_directories(sourcePath.parent_path());

    {
    std::ofstream out(sourcePath, std::ios::binary);
    out << "const char* s = \"hello\";\n";
    out << "line2\n";
    }

    repaddu::core::CliOptions options;
    options.inputPath = repoRoot;
    options.outputPath = outputRoot;
    options.format = repaddu::core::OutputFormat::jsonl;

    std::vector<repaddu::core::FileEntry> files =
        {
        makeCustomEntry(sourcePath, std::filesystem::path("src/escaped.cpp"))
        };

    repaddu::core::OutputChunk chunk;
    chunk.category = "source";
    chunk.title = "source";
    chunk.fileIndices = { 0, 0 };

    const auto result = repaddu::format::writeOutputs(options, files, { chunk }, "", {}, {});
    assert(result.code == repaddu::core::ExitCode::success);

    const std::filesystem::path jsonlPath = outputRoot / "dataset.jsonl";
    assert(std::filesystem::exists(jsonlPath));
    assert(!std::filesystem::exists(outputRoot / "000_overview.md"));

    const std::string content = readText(jsonlPath);
    assert(content.find("\"path\": \"src/escaped.cpp\"") != std::string::npos);
    assert(content.find("\"content\": \"const char* s = \\\"hello\\\";\\nline2\\n\"") != std::string::npos);

    std::size_t lineCount = 0;
    for (char ch : content)
        {
        if (ch == '\n')
            {
            ++lineCount;
            }
        }
    assert(lineCount == 1);
    }

void test_jsonl_dry_run_writes_nothing()
    {
    const std::filesystem::path repoRoot = std::filesystem::path(REPADDU_TEST_ROOT) / "fixtures" / "sample_repo";
    const std::filesystem::path outputRoot = makeTempOutDir("repaddu_jsonl_dry_run");

    repaddu::core::CliOptions options;
    options.inputPath = repoRoot;
    options.outputPath = outputRoot;
    options.format = repaddu::core::OutputFormat::jsonl;
    options.dryRun = true;

    std::vector<repaddu::core::FileEntry> files = { makeEntry(repoRoot) };
    repaddu::core::OutputChunk chunk;
    chunk.category = "source";
    chunk.title = "source";
    chunk.fileIndices = { 0 };

    const auto result = repaddu::format::writeOutputs(options, files, { chunk }, "", {}, {});
    assert(result.code == repaddu::core::ExitCode::success);
    assert(!std::filesystem::exists(outputRoot / "dataset.jsonl"));
    }

int main()
    {
    test_frontmatter_enabled();
    test_frontmatter_disabled();
    test_overview_links_disabled();
    test_jsonl_output_writes_dataset_and_escapes_content();
    test_jsonl_dry_run_writes_nothing();
    std::cout << "Frontmatter output tests passed." << std::endl;
    return 0;
    }
