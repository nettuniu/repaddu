#include "repaddu/logger.h"
#include <fstream>
#include <string>
#include <cassert>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void test_console_logging()
    {
    std::cout << "Testing console logging..." << std::endl;
    // Just ensure it doesn't crash
    repaddu::LogInfo("This is a test INFO message to console");
    repaddu::LogWarn("This is a test WARN message to console");
    }

void test_file_logging()
    {
    std::cout << "Testing file logging..." << std::endl;
    std::string testLogFile = "test_log.txt";
    
    // Clean up previous run
    if (fs::exists(testLogFile))
        {
        fs::remove(testLogFile);
        }

    repaddu::Logger::instance().setLogFile(testLogFile);
    repaddu::Logger::instance().setLevel(repaddu::LogLevel::INFO);

    repaddu::LogInfo("File log entry 1");
    repaddu::LogDebug("Should not appear");
    repaddu::LogError("File log entry 2");

    // Force flush happens on endl in implementation, but we wait a moment or close?
    // The logger keeps file open. We can't easily force close without destructor or new API.
    // But we can check content.

    std::ifstream ifs(testLogFile);
    assert(ifs.is_open());
    
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    // Verify content
    if (content.find("File log entry 1") == std::string::npos)
        {
        std::cerr << "Failed to find 'File log entry 1' in log file" << std::endl;
        exit(1);
        }
    if (content.find("File log entry 2") == std::string::npos)
        {
        std::cerr << "Failed to find 'File log entry 2' in log file" << std::endl;
        exit(1);
        }
    if (content.find("Should not appear") != std::string::npos)
        {
        std::cerr << "Found DEBUG message in INFO level log" << std::endl;
        exit(1);
        }

    // Clean up
    ifs.close();
    // We can't easily delete the file while Logger holds it open (on Windows specifically), 
    // but on Linux it's fine. For correctness, maybe we add a close() method? 
    // Or just leave it for OS cleanup in temp dir.
    }

int main()
    {
    test_console_logging();
    test_file_logging();
    std::cout << "All logging tests passed!" << std::endl;
    return 0;
    }
