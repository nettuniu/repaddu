#ifndef REPADDU_PII_REDACTOR_H
#define REPADDU_PII_REDACTOR_H

#include <string>
#include <vector>
#include <regex>

namespace repaddu::security
    {
    class PiiRedactor
        {
        public:
            PiiRedactor();
            
            // Returns a redacted version of the input string
            std::string redact(const std::string& input, const std::string& filePath = "");

        private:
            struct Pattern
                {
                std::regex regex;
                std::string replacement;
                std::string name;
                };

            std::vector<Pattern> patterns_;
        };
    }

#endif // REPADDU_PII_REDACTOR_H
