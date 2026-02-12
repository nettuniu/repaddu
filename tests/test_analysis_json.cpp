#include "repaddu/format_analysis_json.h"
#include "repaddu/analysis_graph.h"
#include "repaddu/analysis_view.h"

#include <cassert>
#include <filesystem>
#include <iostream>

void test_analysis_json()
    {
    repaddu::core::CliOptions options;
    options.inputPath = "repo";
    options.includeBinaries = false;
    options.analysisEnabled = true;
    options.analysisViews = { "symbols" };

    repaddu::core::FileEntry fileA;
    fileA.relativePath = std::filesystem::path("src/a.cpp");
    fileA.extensionLower = ".cpp";
    fileA.sizeBytes = 100;
    fileA.isBinary = false;

    repaddu::core::FileEntry fileB;
    fileB.relativePath = std::filesystem::path("include/b.hpp");
    fileB.extensionLower = ".hpp";
    fileB.sizeBytes = 40;
    fileB.isBinary = false;

    std::vector<repaddu::core::FileEntry> files = { fileA, fileB };
    std::vector<std::size_t> included = { 0, 1 };

    repaddu::analysis::AnalysisGraph graph;
    repaddu::analysis::SymbolNodeInput cls;
    cls.kind = repaddu::analysis::SymbolKind::class_;
    cls.name = "Widget";
    cls.qualifiedName = "core::Widget";
    cls.containerName = "core";
    cls.isPublic = true;
    graph.addSymbol(cls);

    repaddu::analysis::AnalysisViewOptions viewOptions;
    const std::string json = repaddu::format::renderAnalysisJson(options, files, included, &graph, &viewOptions);

    assert(json.find("\"type\": \"analysis_report\"") != std::string::npos);
    assert(json.find("\"total_files\": 2") != std::string::npos);
    assert(json.find("\"file_count\": 2") != std::string::npos);
    assert(json.find("\"name\": \"C++\"") != std::string::npos);
    assert(json.find("\"count\": 2") != std::string::npos);
    assert(json.find("\"views\"") != std::string::npos);
    assert(json.find("\"nodes\"") != std::string::npos);
    assert(json.find("\"edges\"") != std::string::npos);
    assert(json.find("\"metadata\"") != std::string::npos);
    assert(json.find("\"id\": \"core::Widget\"") != std::string::npos);
    }

int main()
    {
    test_analysis_json();
    std::cout << "Analysis JSON tests passed." << std::endl;
    return 0;
    }
