#ifndef REPADDU_UI_CONSOLE_H
#define REPADDU_UI_CONSOLE_H

#include "repaddu/ui_interface.h"

namespace repaddu::ui
    {
    class ConsoleUI : public UserInterface
        {
        public:
            void logInfo(const std::string& msg) override;
            void logError(const std::string& msg) override;
            void logWarning(const std::string& msg) override;
            
            void startProgress(const std::string& label, int totalSteps) override;
            void updateProgress(int step, const std::string& status) override;
            void endProgress() override;

        private:
            std::string currentLabel_;
            int totalSteps_ = 0;
            int lastPercent_ = -1;
            bool inProgress_ = false;
        };
    }

#endif // REPADDU_UI_CONSOLE_H
