#include "repaddu/analysis_tokens.h"
#include <cassert>
#include <iostream>

void test_token_estimation()
    {
    using repaddu::analysis::TokenEstimator;

    // Empty
    assert(TokenEstimator::estimateTokens("") == 0);

    // Very short (should be at least 1)
    assert(TokenEstimator::estimateTokens("a") == 1);
    assert(TokenEstimator::estimateTokens("abc") == 1);

    // Exact multiple of 4
    assert(TokenEstimator::estimateTokens("abcd") == 1);
    assert(TokenEstimator::estimateTokens("abcdefgh") == 2);

    // Slightly above multiple of 4 (ceil)
    assert(TokenEstimator::estimateTokens("abcde") == 2);
    
    // Large string
    std::string large(400, 'x');
    assert(TokenEstimator::estimateTokens(large) == 100);

    std::cout << "Token estimation tests passed." << std::endl;
    }

int main()
    {
    test_token_estimation();
    return 0;
    }
