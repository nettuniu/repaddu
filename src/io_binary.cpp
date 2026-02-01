#include "repaddu/io_binary.h"

#include <array>
#include <fstream>

namespace repaddu::io
    {
    bool looksBinary(const std::filesystem::path& filePath)
        {
        std::ifstream stream(filePath, std::ios::binary);
        if (!stream)
            {
            return false;
            }

        std::array<char, 4096> buffer{};
        stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize readCount = stream.gcount();
        for (std::streamsize i = 0; i < readCount; ++i)
            {
            if (buffer[static_cast<std::size_t>(i)] == '\0')
                {
                return true;
                }
            }
        return false;
        }
    }
