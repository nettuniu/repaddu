#include "repaddu/ui_plain.h"

#include <iostream>

namespace repaddu::ui
    {
    void PlainUI::logInfo(const std::string& msg)
        {
        std::cout << "[INFO] " << msg << std::endl;
        }

    void PlainUI::logError(const std::string& msg)
        {
        std::cerr << "[ERROR] " << msg << std::endl;
        }

    void PlainUI::logWarning(const std::string& msg)
        {
        std::cerr << "[WARN] " << msg << std::endl;
        }

    void PlainUI::startProgress(const std::string& label, int totalSteps)
        {
        (void)label;
        (void)totalSteps;
        }

    void PlainUI::updateProgress(int step, const std::string& status)
        {
        (void)step;
        (void)status;
        }

    void PlainUI::endProgress()
        {
        }
    }
