#include "repaddu/language_profiles.h"

#include <cassert>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

namespace
    {
    repaddu::core::FileEntry makeFile(const std::filesystem::path& path,
        const std::string& extensionLower, bool isBinary = false)
        {
        repaddu::core::FileEntry entry;
        entry.relativePath = path;
        entry.extensionLower = extensionLower;
        entry.isBinary = isBinary;
        return entry;
        }
    }

void test_detect_rust_and_cargo()
    {
    const std::vector<repaddu::core::FileEntry> files =
        {
            makeFile("src/main.rs", ".rs"),
            makeFile("Cargo.toml", ".toml"),
            makeFile("Cargo.lock", ".lock")
        };

    const repaddu::core::DetectionResult detected = repaddu::core::detectLanguageAndBuildSystem(files);
    assert(detected.languageId == "rust");
    assert(detected.buildSystemId == "cargo");
    }

void test_detect_cpp_precedence_on_tie()
    {
    const std::vector<repaddu::core::FileEntry> files =
        {
            makeFile("src/main.c", ".c"),
            makeFile("src/main.cpp", ".cpp")
        };

    const repaddu::core::DetectionResult detected = repaddu::core::detectLanguageAndBuildSystem(files);
    assert(detected.languageId == "cpp");
    assert(detected.buildSystemId.empty());
    }

void test_detect_none_when_unknown()
    {
    const std::vector<repaddu::core::FileEntry> files =
        {
            makeFile("notes/todo.txt", ".txt"),
            makeFile("assets/logo.bin", ".bin", true)
        };

    const repaddu::core::DetectionResult detected = repaddu::core::detectLanguageAndBuildSystem(files);
    assert(detected.languageId.empty());
    assert(detected.buildSystemId.empty());
    }

void test_build_system_precedence()
    {
    const std::vector<repaddu::core::FileEntry> files =
        {
            makeFile("CMakeLists.txt", ".txt"),
            makeFile("Cargo.toml", ".toml")
        };

    const repaddu::core::DetectionResult detected = repaddu::core::detectLanguageAndBuildSystem(files);
    assert(detected.buildSystemId == "cargo");
    }

void test_detect_npm_build_system()
    {
    const std::vector<repaddu::core::FileEntry> files =
        {
            makeFile("package.json", ".json"),
            makeFile("index.js", ".js")
        };

    const repaddu::core::DetectionResult detected = repaddu::core::detectLanguageAndBuildSystem(files);
    assert(detected.buildSystemId == "npm");
    }

void test_detect_npm_build_system_case_insensitive_filename()
    {
    const std::vector<repaddu::core::FileEntry> files =
        {
            makeFile("Package.JSON", ".json"),
            makeFile("index.js", ".js")
        };

    const repaddu::core::DetectionResult detected = repaddu::core::detectLanguageAndBuildSystem(files);
    assert(detected.buildSystemId == "npm");
    }

void test_resolve_build_files_default_includes_known_systems()
    {
    repaddu::core::CliOptions options;
    const std::vector<std::string> names = repaddu::core::resolveBuildFileNames(options);

    assert(std::find(names.begin(), names.end(), "CMakeLists.txt") != names.end());
    assert(std::find(names.begin(), names.end(), "package.json") != names.end());
    assert(std::find(names.begin(), names.end(), "pyproject.toml") != names.end());
    }

void test_resolve_build_files_deduplicates_language_and_build()
    {
    repaddu::core::CliOptions options;
    options.language = "python";
    options.buildSystem = "python";

    const std::vector<std::string> names = repaddu::core::resolveBuildFileNames(options);
    const std::size_t pyprojectCount = static_cast<std::size_t>(
        std::count(names.begin(), names.end(), "pyproject.toml"));
    const std::size_t setupPyCount = static_cast<std::size_t>(
        std::count(names.begin(), names.end(), "setup.py"));
    assert(pyprojectCount == 1);
    assert(setupPyCount == 1);
    }

void test_resolve_build_files_unions_build_system_and_language()
    {
    repaddu::core::CliOptions options;
    options.language = "cpp";
    options.buildSystem = "npm";

    const std::vector<std::string> names = repaddu::core::resolveBuildFileNames(options);
    assert(std::find(names.begin(), names.end(), "CMakeLists.txt") != names.end());
    assert(std::find(names.begin(), names.end(), "package.json") != names.end());
    }

void test_resolve_build_files_unknown_ids_fall_back_to_default_set()
    {
    repaddu::core::CliOptions options;
    options.language = "unknown-language";
    options.buildSystem = "unknown-build-system";

    const std::vector<std::string> names = repaddu::core::resolveBuildFileNames(options);
    assert(std::find(names.begin(), names.end(), "CMakeLists.txt") != names.end());
    assert(std::find(names.begin(), names.end(), "Cargo.toml") != names.end());
    assert(std::find(names.begin(), names.end(), "package.json") != names.end());
    }

int main()
    {
    test_detect_rust_and_cargo();
    test_detect_cpp_precedence_on_tie();
    test_detect_none_when_unknown();
    test_build_system_precedence();
    test_detect_npm_build_system();
    test_detect_npm_build_system_case_insensitive_filename();
    test_resolve_build_files_default_includes_known_systems();
    test_resolve_build_files_deduplicates_language_and_build();
    test_resolve_build_files_unions_build_system_and_language();
    test_resolve_build_files_unknown_ids_fall_back_to_default_set();
    std::cout << "Language/build auto-detection tests passed." << std::endl;
    return 0;
    }
