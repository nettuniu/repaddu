#ifndef REPADDU_UI_INTERFACE_H
#define REPADDU_UI_INTERFACE_H

#include <string>

namespace repaddu::ui
    {
    class UserInterface
        {
        public:
            virtual ~UserInterface() = default;

            virtual void logInfo(const std::string& msg) = 0;
            virtual void logError(const std::string& msg) = 0;
            virtual void logWarning(const std::string& msg) = 0;
            
            // Progress reporting
            virtual void startProgress(const std::string& label, int totalSteps) = 0;
            virtual void updateProgress(int step, const std::string& status = "") = 0;
            virtual void endProgress() = 0;
        };
    }

#endif // REPADDU_UI_INTERFACE_H
