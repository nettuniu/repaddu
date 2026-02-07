#include "repaddu/ui_console.h"
#include <thread>
#include <chrono>
#include <iostream>

void test_progress_bar()
    {
    std::cout << "Testing Progress Bar (Visual Check)..." << std::endl;
    repaddu::ui::ConsoleUI ui;
    
    ui.logInfo("Starting task...");
    ui.startProgress("Processing files", 10);
    
    for (int i = 0; i <= 10; ++i)
        {
        ui.updateProgress(i, "file_" + std::to_string(i) + ".txt");
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep to make it visible
        if (i == 5)
            {
            ui.logWarning("Halfway warning!");
            }
        }
    
    ui.endProgress();
    std::cout << "Progress Bar Test Complete." << std::endl;
    }

int main()
    {
    test_progress_bar();
    return 0;
    }
