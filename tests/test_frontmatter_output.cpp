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

int main()
    {
    test_frontmatter_enabled();
    test_frontmatter_disabled();
    std::cout << "Frontmatter output tests passed." << std::endl;
    return 0;
    }
