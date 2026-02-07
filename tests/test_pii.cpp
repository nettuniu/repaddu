#include "repaddu/pii_redactor.h"
#include <cassert>
#include <iostream>
#include <string>

void test_email_redaction()
    {
    repaddu::security::PiiRedactor redactor;
    std::string input = "Contact me at user@example.com for details.";
    std::string expected = "Contact me at <REDACTED:EMAIL> for details.";
    std::string output = redactor.redact(input);
    
    if (output != expected)
        {
        std::cerr << "Email redaction failed.\nExpected: " << expected << "\nActual:   " << output << std::endl;
        exit(1);
        }
    std::cout << "Email redaction passed." << std::endl;
    }

void test_ip_redaction()
    {
    repaddu::security::PiiRedactor redactor;
    std::string input = "Server IP is 192.168.1.1 connected.";
    std::string expected = "Server IP is <REDACTED:IPV4> connected.";
    std::string output = redactor.redact(input);

    if (output != expected)
        {
        std::cerr << "IP redaction failed.\nExpected: " << expected << "\nActual:   " << output << std::endl;
        exit(1);
        }
    std::cout << "IP redaction passed." << std::endl;
    }

void test_github_token()
    {
    repaddu::security::PiiRedactor redactor;
    std::string input = "Token: ghp_123456789012345678901234567890123456";
    std::string expected = "Token: <REDACTED:GITHUB_TOKEN>";
    std::string output = redactor.redact(input);

    if (output != expected)
        {
        std::cerr << "GitHub token redaction failed.\nExpected: " << expected << "\nActual:   " << output << std::endl;
        exit(1);
        }
    std::cout << "GitHub token redaction passed." << std::endl;
    }

int main()
    {
    test_email_redaction();
    test_ip_redaction();
    test_github_token();
    std::cout << "All PII redaction tests passed!" << std::endl;
    return 0;
    }
