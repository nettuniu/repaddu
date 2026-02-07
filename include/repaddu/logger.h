#ifndef REPADDU_LOGGER_H
#define REPADDU_LOGGER_H

#include <string>
#include <mutex>
#include <fstream>
#include <iostream>

namespace repaddu
    {
    enum class LogLevel
        {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR,
        NONE
        };

    class Logger
        {
        public:
            static Logger& instance();

            // Delete copy/move
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;

            void setLevel(LogLevel level);
            void setLogFile(const std::string& path);
            void log(LogLevel level, const std::string& message);

        private:
            Logger() = default;
            ~Logger();

            void writeLog(LogLevel level, const std::string& message);
            std::string getTimestamp();
            std::string levelToString(LogLevel level);

            LogLevel currentLevel_ = LogLevel::INFO;
            std::ofstream logFile_;
            std::mutex mutex_;
        };

    // Helper functions (or macros could be used, but inline functions are cleaner)
    inline void LogDebug(const std::string& msg) { Logger::instance().log(LogLevel::DEBUG, msg); }
    inline void LogInfo(const std::string& msg) { Logger::instance().log(LogLevel::INFO, msg); }
    inline void LogWarn(const std::string& msg) { Logger::instance().log(LogLevel::WARN, msg); }
    inline void LogError(const std::string& msg) { Logger::instance().log(LogLevel::ERROR, msg); }

    } // namespace repaddu

#endif // REPADDU_LOGGER_H
