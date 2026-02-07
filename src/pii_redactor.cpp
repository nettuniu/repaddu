#include "repaddu/pii_redactor.h"
#include "repaddu/logger.h"

#include <iostream>

namespace repaddu::security
    {
    PiiRedactor::PiiRedactor()
        {
        // Email
        patterns_.push_back({
            std::regex(R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}\b)"),
            "<REDACTED:EMAIL>",
            "Email Address"
        });

        // IPv4 (Simplified)
        patterns_.push_back({
            std::regex(R"(\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b)"),
            "<REDACTED:IPV4>",
            "IPv4 Address"
        });

        // GitHub Personal Access Token
        patterns_.push_back({
            std::regex(R"(ghp_[a-zA-Z0-9]{36})"),
            "<REDACTED:GITHUB_TOKEN>",
            "GitHub Token"
        });

        // AWS Access Key ID
        patterns_.push_back({
            std::regex(R"((?:AKIA|ASIA)[0-9A-Z]{16})"),
            "<REDACTED:AWS_KEY>",
            "AWS Access Key"
        });
        
        // Generic "API Key" assignment (conservative)
        // Look for "api_key = '...'"
        patterns_.push_back({
            std::regex(R"((api_key|secret_key|auth_token)\s*[:=]\s*['"]([a-zA-Z0-9_\-]{20,})['"])"),
            "$1 = <REDACTED:SECRET>", // Preserves the key name
            "Generic Secret Assignment"
        });
        }

    std::string PiiRedactor::redact(const std::string& input, const std::string& filePath)
        {
        std::string result = input;
        bool modified = false;

        for (const auto& pattern : patterns_)
            {
            // We use a callback approach or just regex_replace?
            // regex_replace replaces ALL occurrences.
            // We also want to log if we found something.
            
            if (std::regex_search(result, pattern.regex))
                {
                // We found matches.
                // To get specific matches for logging, we can iterate. 
                // But for performance, maybe we just replace and verify change.
                
                // Let's iterate to log "Found X in file Y"
                auto begin = std::sregex_iterator(result.begin(), result.end(), pattern.regex);
                auto end = std::sregex_iterator();
                
                for (std::sregex_iterator i = begin; i != end; ++i)
                    {
                    std::smatch match = *i;
                    std::string matchStr = match.str();
                    // Avoid logging the actual secret if possible, or just log "Found GitHub Token"
                    LogWarn("PII/Secret detected (" + pattern.name + ") in " + 
                            (filePath.empty() ? "content" : filePath));
                    }

                // If the pattern is the generic one with groups, we need to handle format differently
                if (pattern.name == "Generic Secret Assignment")
                    {
                    result = std::regex_replace(result, pattern.regex, "$1 = \"<REDACTED:SECRET>\"");
                    }
                else
                    {
                    result = std::regex_replace(result, pattern.regex, pattern.replacement);
                    }
                }
            }
        
        return result;
        }
    }
