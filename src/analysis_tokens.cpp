#include "repaddu/analysis_tokens.h"

#include <cmath>

namespace repaddu::analysis
    {
    std::uintmax_t TokenEstimator::estimateTokens(const std::string& content)
        {
        if (content.empty())
            {
            return 0;
            }

        // Standard heuristic: 1 token ~= 4 characters for code/text.
        // We use double to avoid integer truncation before the final cast.
        double estimated = static_cast<double>(content.size()) / 4.0;
        
        // Ensure at least 1 token if content is not empty
        if (estimated < 1.0)
            {
            return 1;
            }
            
        return static_cast<std::uintmax_t>(std::ceil(estimated));
        }
    }
