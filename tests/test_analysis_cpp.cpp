#include "repaddu/analysis_cpp.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
    {
    std::filesystem::path fixtureRoot()
        {
        const std::filesystem::path root = REPADDU_TEST_ROOT;
        return root / "fixtures" / "analysis_cpp";
        }

    std::filesystem::path prepareWorkDir()
        {
        const std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "repaddu_cpp_analysis_fixture";
        std::filesystem::remove_all(tempDir);
        std::filesystem::create_directories(tempDir);

        const std::filesystem::path fixtureDir = fixtureRoot();
        const std::filesystem::path sourcePath = tempDir / "sample.cpp";
        std::filesystem::copy_file(fixtureDir / "sample.cpp", sourcePath,
            std::filesystem::copy_options::overwrite_existing);

        const std::filesystem::path compileCommandsPath = tempDir / "compile_commands.json";
        std::ofstream compileCommands(compileCommandsPath);
        compileCommands << "[\n";
        compileCommands << "  {\n";
        compileCommands << "    \"directory\": \"" << tempDir.string() << "\",\n";
        compileCommands << "    \"command\": \"clang++ -std=c++20 -c " << sourcePath.string() << "\",\n";
        compileCommands << "    \"file\": \"" << sourcePath.string() << "\"\n";
        compileCommands << "  }\n";
        compileCommands << "]\n";

        return tempDir;
        }
    }

void test_cpp_analysis()
    {
    repaddu::analysis::AnalysisGraph graph;
    const auto dir = prepareWorkDir();

    repaddu::analysis::CppAnalysisOptions options;
    options.compileCommandsPath = dir;
    options.deep = true;

    const auto result = repaddu::analysis::analyzeCppProject(options, graph);
    assert(result.code == repaddu::core::ExitCode::success);

    const auto* cls = graph.findSymbolByQualifiedName("sample::Base");
    assert(cls != nullptr);

    const auto* method = graph.findSymbolByQualifiedName("sample::Base::run");
    assert(method != nullptr);

    const auto* derived = graph.findSymbolByQualifiedName("sample::Derived");
    assert(derived != nullptr);

    const auto* derivedMethod = graph.findSymbolByQualifiedName("sample::Derived::run");
    assert(derivedMethod != nullptr);

    bool sawInherits = false;
    bool sawOverrides = false;
    bool sawImplementedBy = false;
    for (const auto& edge : graph.edges())
        {
        const auto& from = graph.getSymbol(edge.from);
        const auto& to = graph.getSymbol(edge.to);
        if (from.qualifiedName == "sample::Derived" && to.qualifiedName == "sample::Base"
            && edge.kind == repaddu::analysis::EdgeKind::inherits)
            {
            sawInherits = true;
            }
        if (from.qualifiedName == "sample::Derived::run" && to.qualifiedName == "sample::Base::run"
            && edge.kind == repaddu::analysis::EdgeKind::overrides)
            {
            sawOverrides = true;
            }
        if (from.qualifiedName == "sample::Base::run" && to.qualifiedName == "sample::Derived::run"
            && edge.kind == repaddu::analysis::EdgeKind::implemented_by)
            {
            sawImplementedBy = true;
            }
        }

    assert(sawInherits);
    assert(sawOverrides);
    assert(sawImplementedBy);
    }

void test_cpp_analysis_without_deep_edges()
    {
    repaddu::analysis::AnalysisGraph graph;
    const auto dir = prepareWorkDir();

    repaddu::analysis::CppAnalysisOptions options;
    options.compileCommandsPath = dir;
    options.deep = false;

    const auto result = repaddu::analysis::analyzeCppProject(options, graph);
    assert(result.code == repaddu::core::ExitCode::success);

    bool sawInherits = false;
    bool sawOverrides = false;
    bool sawImplementedBy = false;
    for (const auto& edge : graph.edges())
        {
        const auto& from = graph.getSymbol(edge.from);
        const auto& to = graph.getSymbol(edge.to);
        if (from.qualifiedName == "sample::Derived" && to.qualifiedName == "sample::Base"
            && edge.kind == repaddu::analysis::EdgeKind::inherits)
            {
            sawInherits = true;
            }
        if (edge.kind == repaddu::analysis::EdgeKind::overrides)
            {
            sawOverrides = true;
            }
        if (edge.kind == repaddu::analysis::EdgeKind::implemented_by)
            {
            sawImplementedBy = true;
            }
        }

    assert(sawInherits);
    assert(!sawOverrides);
    assert(!sawImplementedBy);
    }

int main()
    {
    test_cpp_analysis();
    test_cpp_analysis_without_deep_edges();
    std::cout << "C++ analysis tests passed." << std::endl;
    return 0;
    }
