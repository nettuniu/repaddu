#include "repaddu/ui_console.h"
#include <iostream>
#include <iomanip>
#include <cmath>

namespace repaddu::ui
    {
    void ConsoleUI::logInfo(const std::string& msg)
        {
        if (inProgress_)
            {
            // Clear current progress line before logging
            std::cout << "\r\033[K"; 
            }
        std::cout << "[INFO] " << msg << std::endl;
        if (inProgress_)
            {
            // Redraw progress (simplified: just force update next time or leave blank)
            // Ideally we'd redraw the bar, but for now let's just let the next update handle it.
            }
        }

    void ConsoleUI::logError(const std::string& msg)
        {
        if (inProgress_) std::cerr << "\r\033[K";
        std::cerr << "[ERROR] " << msg << std::endl;
        }

    void ConsoleUI::logWarning(const std::string& msg)
        {
        if (inProgress_) std::cerr << "\r\033[K";
        std::cerr << "[WARN] " << msg << std::endl;
        }

    void ConsoleUI::startProgress(const std::string& label, int totalSteps)
        {
        currentLabel_ = label;
        totalSteps_ = totalSteps;
        lastPercent_ = -1;
        inProgress_ = true;
        updateProgress(0, "");
        }

    void ConsoleUI::updateProgress(int step, const std::string& status)
        {
        if (!inProgress_) return;

        int percent = 0;
        if (totalSteps_ > 0)
            {
            percent = static_cast<int>((static_cast<double>(step) / totalSteps_) * 100.0);
            }
        else
            {
            // Indeterminate? For now just show step count
            }

        // Avoid flickering if no change, unless we need to clear artifacts
        // if (percent == lastPercent_ && status.empty()) return; 
        lastPercent_ = percent;

        // Bar width
        const int barWidth = 30;
        
        std::cout << "\r\033[K"; // Carriage return and clear line
        std::cout << currentLabel_ << " [";
        
        if (totalSteps_ > 0)
            {
            int pos = static_cast<int>(barWidth * (static_cast<double>(step) / totalSteps_));
            for (int i = 0; i < barWidth; ++i)
                {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
                }
            std::cout << "] " << percent << "%";
            }
        else
            {
            // Indeterminate spinner or just count
            std::cout << "..............................] " << step;
            }

        if (!status.empty())
            {
            std::cout << " " << status;
            }
        
        std::cout << std::flush;
        }

    void ConsoleUI::endProgress()
        {
        if (inProgress_)
            {
            std::cout << "\r\033[K" << currentLabel_ << " Done." << std::endl;
            inProgress_ = false;
            }
        }
    }