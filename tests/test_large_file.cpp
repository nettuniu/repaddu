#include "repaddu/grouping_strategies.h"
#include "repaddu/core_types.h"
#include <vector>
#include <string>
#include <cassert>
#include <iostream>

void test_large_file_exclusion()
    {
    repaddu::core::CliOptions options;
    options.maxFileSize = 1024; // 1KB limit for testing
    options.forceLargeFiles = false;
    // We need to set extensions to empty or something to allow all, or specific
    options.extensions = { ".txt" }; 

    repaddu::core::FileEntry smallFile;
    smallFile.relativePath = "small.txt";
    smallFile.extensionLower = ".txt";
    smallFile.sizeBytes = 500;
    smallFile.isBinary = false;

    repaddu::core::FileEntry largeFile;
    largeFile.relativePath = "large.txt";
    largeFile.extensionLower = ".txt";
    largeFile.sizeBytes = 2000;
    largeFile.isBinary = false;

    std::vector<repaddu::core::FileEntry> files = { smallFile, largeFile };
    repaddu::core::RunResult result;
    
    // 1. Test Default (Exclude)
    auto groupingResult = repaddu::grouping::filterAndGroupFiles(options, files, nullptr, result);
    
    assert(groupingResult.includedIndices.size() == 1);
    assert(groupingResult.includedIndices[0] == 0); // Only small file
    std::cout << "Large file exclusion passed." << std::endl;

    // 2. Test Force (Include)
    options.forceLargeFiles = true;
    groupingResult = repaddu::grouping::filterAndGroupFiles(options, files, nullptr, result);
    
    assert(groupingResult.includedIndices.size() == 2);
    std::cout << "Large file forced inclusion passed." << std::endl;
    }

int main()
    {
    test_large_file_exclusion();
    return 0;
    }
