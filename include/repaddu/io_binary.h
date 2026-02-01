#ifndef REPADDU_IO_BINARY_H
#define REPADDU_IO_BINARY_H

#include <filesystem>

namespace repaddu::io
    {
    bool looksBinary(const std::filesystem::path& filePath);
    }

#endif // REPADDU_IO_BINARY_H
