#include "repaddu/logger.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace repaddu
    {
    Logger& Logger::instance()
        {
        static Logger instance;
        return instance;
        }

    Logger::~Logger()
        {
        if (logFile_.is_open())
            {
            logFile_.close();
            }
        }

    void Logger::setLevel(LogLevel level)
        {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel_ = level;
        }

    void Logger::setLogFile(const std::string& path)
        {
        std::lock_guard<std::mutex> lock(mutex_);
        if (logFile_.is_open())
            {
            logFile_.close();
            }
        logFile_.open(path, std::ios::out | std::ios::app);
        }

    void Logger::log(LogLevel level, const std::string& message)
        {
        if (level < currentLevel_)
            {
            return;
            }

        std::lock_guard<std::mutex> lock(mutex_);
        writeLog(level, message);
        }

    void Logger::writeLog(LogLevel level, const std::string& message)
        {
        std::stringstream ss;
        ss << "[" << getTimestamp() << "] "
           << "[" << levelToString(level) << "] "
           << message;

        std::string finalMsg = ss.str();

        // Always write to standard error for visibility (or stdout for info?) 
        // Typically logs go to stderr to not pollute stdout piping.
        std::cerr << finalMsg << std::endl;

        if (logFile_.is_open())
            {
            logFile_ << finalMsg << std::endl;
            }
        }

    std::string Logger::getTimestamp()
        {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        return ss.str();
        }

    std::string Logger::levelToString(LogLevel level)
        {
        switch (level)
            {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERROR: return "ERROR";
            default:              return "UNKNOWN";
            }
        }
    }
