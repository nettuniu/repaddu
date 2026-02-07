#ifndef REPADDU_ANALYSIS_TOKENS_H
#define REPADDU_ANALYSIS_TOKENS_H

#include <string>
#include <cstdint>

namespace repaddu::analysis
    {
    class TokenEstimator
        {
        public:
            // Heuristic-based estimation: ~4 chars per token for English/Code
            // This is a common baseline used when a real tokenizer isn't linked.
            static std::uintmax_t estimateTokens(const std::string& content);
        };
    }

#endif // REPADDU_ANALYSIS_TOKENS_H
