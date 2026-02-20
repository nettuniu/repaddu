#ifndef REPADDU_UI_PLAIN_H
#define REPADDU_UI_PLAIN_H

#include "repaddu/ui_interface.h"

namespace repaddu::ui
    {
    class PlainUI : public UserInterface
        {
        public:
            void logInfo(const std::string& msg) override;
            void logError(const std::string& msg) override;
            void logWarning(const std::string& msg) override;

            void startProgress(const std::string& label, int totalSteps) override;
            void updateProgress(int step, const std::string& status) override;
            void endProgress() override;
        };
    }

#endif // REPADDU_UI_PLAIN_H
