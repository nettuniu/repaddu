#include "repaddu/language_profiles.h"

#include <cassert>
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

int main()
    {
    test_detect_rust_and_cargo();
    test_detect_cpp_precedence_on_tie();
    test_detect_none_when_unknown();
    test_build_system_precedence();
    std::cout << "Language/build auto-detection tests passed." << std::endl;
    return 0;
    }
