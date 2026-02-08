#include "repaddu/analysis_cpp.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
    {
    std::filesystem::path prepareFixture()
        {
        std::filesystem::path dir = std::filesystem::temp_directory_path() / "repaddu_cpp_analysis_fixture";
        std::filesystem::create_directories(dir);

        const std::filesystem::path sourcePath = dir / "sample.cpp";
        std::ofstream source(sourcePath);
        source << "namespace sample {\n";
        source << "class Base {\n";
        source << "public:\n";
        source << "    virtual void run() = 0;\n";
        source << "};\n";
        source << "class Derived : public Base {\n";
        source << "public:\n";
        source << "    void run() override {}\n";
        source << "};\n";
        source << "}\n";

        const std::filesystem::path compileCommands = dir / "compile_commands.json";
        std::ofstream commands(compileCommands);
        commands << "[\n";
        commands << "  {\n";
        commands << "    \"directory\": \"" << dir.string() << "\",\n";
        commands << "    \"command\": \"clang++ -std=c++20 -c " << sourcePath.string() << "\",\n";
        commands << "    \"file\": \"" << sourcePath.string() << "\"\n";
        commands << "  }\n";
        commands << "]\n";

        return dir;
        }
    }

void test_cpp_analysis()
    {
    repaddu::analysis::AnalysisGraph graph;
    const auto dir = prepareFixture();

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

int main()
    {
    test_cpp_analysis();
    std::cout << "C++ analysis tests passed." << std::endl;
    return 0;
    }
