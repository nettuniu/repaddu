#ifndef REPADDU_IO_ZIP_H
#define REPADDU_IO_ZIP_H

#include "repaddu/core_types.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace repaddu::io
    {
    enum class CompressionMethod : std::uint16_t
        {
        store = 0,
        deflate = 8
        };

    struct ZipEntry
        {
        std::string archivePath;
        std::vector<std::uint8_t> data;
        };

    core::RunResult writeZipArchive(const std::filesystem::path& zipPath,
        const std::vector<ZipEntry>& entries,
        CompressionMethod method = CompressionMethod::deflate);

    core::RunResult createZipFromDirectory(const std::filesystem::path& sourceDirectory,
        const std::filesystem::path& zipPath,
        CompressionMethod method = CompressionMethod::deflate);
    }

#endif // REPADDU_IO_ZIP_H
