#include "repaddu/io_binary.h"
#include "repaddu/logger.h"

#include <array>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

namespace repaddu::io
    {
    struct MagicSig
        {
        std::vector<uint8_t> bytes;
        std::string name;
        };

    // Common magic bytes
    static const std::vector<MagicSig> kSignatures = 
        {
        { {0x7f, 0x45, 0x4c, 0x46}, "ELF Executable" },
        { {0x4d, 0x5a}, "PE Executable (Windows)" },
        { {0xfe, 0xed, 0xfa, 0xcf}, "Mach-O (Mac 64)" },
        { {0xfe, 0xed, 0xfa, 0xce}, "Mach-O (Mac 32)" },
        { {0xca, 0xfe, 0xba, 0xbe}, "Java Class / Mach-O Fat" },
        { {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a}, "PNG Image" },
        { {0xff, 0xd8, 0xff}, "JPEG Image" },
        { {0x47, 0x49, 0x46, 0x38}, "GIF Image" },
        { {0x50, 0x4b, 0x03, 0x04}, "ZIP Archive" },
        { {0x1f, 0x8b}, "GZIP Archive" },
        { {0x42, 0x4d}, "BMP Image" },
        { {0x00, 0x00, 0x01, 0x00}, "ICO Icon" } 
        // Add more as needed
        };

    bool looksBinary(const std::filesystem::path& filePath)
        {
        std::ifstream stream(filePath, std::ios::binary);
        if (!stream)
            {
            return false;
            }

        std::array<char, 4096> buffer{};
        stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        std::streamsize readCount = stream.gcount();
        
        if (readCount == 0) return false; // Empty file is text-safe usually

        // 1. Check Magic Signatures
        for (const auto& sig : kSignatures)
            {
            if (readCount >= static_cast<std::streamsize>(sig.bytes.size()))
                {
                bool match = true;
                for (size_t i = 0; i < sig.bytes.size(); ++i)
                    {
                    if (static_cast<uint8_t>(buffer[i]) != sig.bytes[i])
                        {
                        match = false;
                        break;
                        }
                    }
                
                if (match)
                    {
                    LogInfo("Binary detected via magic bytes (" + sig.name + "): " + filePath.string());
                    return true;
                    }
                }
            }

        // 2. Check for NUL bytes (fallback)
        // Heuristic: If more than N nul bytes or nul bytes in first K chars?
        // Simple heuristic: any NUL in first 4KB means binary (except UTF-16? we assume source is UTF-8/ASCII for now)
        for (std::streamsize i = 0; i < readCount; ++i)
            {
            if (buffer[static_cast<std::size_t>(i)] == '\0')
                {
                LogInfo("Binary detected via NUL byte check: " + filePath.string());
                return true;
                }
            }
        return false;
        }
    }
