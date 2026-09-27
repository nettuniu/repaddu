#include "repaddu/io_zip.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <fstream>

namespace repaddu::io
    {
    namespace
        {
        inline void writeUInt16LE(char* dest, std::uint16_t value)
            {
            dest[0] = static_cast<char>(value & 0xFF);
            dest[1] = static_cast<char>((value >> 8) & 0xFF);
            }

        inline void writeUInt32LE(char* dest, std::uint32_t value)
            {
            dest[0] = static_cast<char>(value & 0xFF);
            dest[1] = static_cast<char>((value >> 8) & 0xFF);
            dest[2] = static_cast<char>((value >> 16) & 0xFF);
            dest[3] = static_cast<char>((value >> 24) & 0xFF);
            }

        std::uint32_t calculateCrc32(const std::uint8_t* data, std::size_t length)
            {
            std::uint32_t crc = 0xFFFFFFFF;
            for (std::size_t i = 0; i < length; ++i)
                {
                crc ^= data[i];
                for (int j = 0; j < 8; ++j)
                    {
                    crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
                    }
                }
            return ~crc;
            }

        struct BitWriter
            {
            std::vector<std::uint8_t> buffer;
            std::uint32_t bitBuf = 0;
            int bitCount = 0;

            void writeBits(std::uint32_t value, int count)
                {
                bitBuf |= (value << bitCount);
                bitCount += count;
                while (bitCount >= 8)
                    {
                    buffer.push_back(static_cast<std::uint8_t>(bitBuf & 0xFF));
                    bitBuf >>= 8;
                    bitCount -= 8;
                    }
                }

            void flush()
                {
                if (bitCount > 0)
                    {
                    buffer.push_back(static_cast<std::uint8_t>(bitBuf & 0xFF));
                    bitBuf = 0;
                    bitCount = 0;
                    }
                }
            };

        inline std::uint16_t reverseBits(std::uint16_t code, int n)
            {
            std::uint16_t res = 0;
            for (int i = 0; i < n; ++i)
                {
                if (code & (1 << i))
                    {
                    res |= (1 << (n - 1 - i));
                    }
                }
            return res;
            }

        inline void writeFixedLitOrLen(BitWriter& bw, std::uint16_t code)
            {
            if (code <= 143)
                {
                bw.writeBits(reverseBits(0x30 + code, 8), 8);
                }
            else if (code <= 255)
                {
                bw.writeBits(reverseBits(0x190 + (code - 144), 9), 9);
                }
            else if (code <= 279)
                {
                bw.writeBits(reverseBits(code - 256, 7), 7);
                }
            else if (code <= 287)
                {
                bw.writeBits(reverseBits(0xC0 + (code - 280), 8), 8);
                }
            }

        inline void writeFixedDistance(BitWriter& bw, std::uint16_t distCode)
            {
            bw.writeBits(reverseBits(distCode, 5), 5);
            }

        struct LengthDef
            {
            std::uint16_t code;
            std::uint8_t extraBits;
            std::uint16_t baseLength;
            };

        static const LengthDef kLengthTable[] =
            {
            {257, 0, 3}, {258, 0, 4}, {259, 0, 5}, {260, 0, 6},
            {261, 0, 7}, {262, 0, 8}, {263, 0, 9}, {264, 0, 10},
            {265, 1, 11}, {266, 1, 13}, {267, 1, 15}, {268, 1, 17},
            {269, 2, 19}, {270, 2, 23}, {271, 2, 27}, {272, 2, 31},
            {273, 3, 35}, {274, 3, 43}, {275, 3, 51}, {276, 3, 59},
            {277, 4, 67}, {278, 4, 83}, {279, 4, 99}, {280, 4, 115},
            {281, 5, 131}, {282, 5, 163}, {283, 5, 195}, {284, 5, 227},
            {285, 0, 258}
            };

        inline void encodeLength(BitWriter& bw, std::uint16_t length)
            {
            int idx = 0;
            while (idx < 28 && length >= kLengthTable[idx + 1].baseLength)
                {
                ++idx;
                }
            const auto& item = kLengthTable[idx];
            writeFixedLitOrLen(bw, item.code);
            if (item.extraBits > 0)
                {
                const std::uint16_t extra = length - item.baseLength;
                bw.writeBits(extra, item.extraBits);
                }
            }

        struct DistDef
            {
            std::uint8_t code;
            std::uint8_t extraBits;
            std::uint16_t baseDist;
            };

        static const DistDef kDistTable[] =
            {
            {0, 0, 1}, {1, 0, 2}, {2, 0, 3}, {3, 0, 4},
            {4, 1, 5}, {5, 1, 7}, {6, 2, 9}, {7, 2, 13},
            {8, 3, 17}, {9, 3, 25}, {10, 4, 33}, {11, 4, 49},
            {12, 5, 65}, {13, 5, 97}, {14, 6, 129}, {15, 6, 193},
            {16, 7, 257}, {17, 7, 385}, {18, 8, 513}, {19, 8, 769},
            {20, 9, 1025}, {21, 9, 1537}, {22, 10, 2049}, {23, 10, 3073},
            {24, 11, 4097}, {25, 11, 6145}, {26, 12, 8193}, {27, 12, 12289},
            {28, 13, 16385}, {29, 13, 24577}
            };

        inline void encodeDistance(BitWriter& bw, std::uint16_t distance)
            {
            int idx = 0;
            while (idx < 29 && distance >= kDistTable[idx + 1].baseDist)
                {
                ++idx;
                }
            const auto& item = kDistTable[idx];
            writeFixedDistance(bw, item.code);
            if (item.extraBits > 0)
                {
                const std::uint16_t extra = distance - item.baseDist;
                bw.writeBits(extra, item.extraBits);
                }
            }

        std::vector<std::uint8_t> compressDeflate(const std::uint8_t* data, std::size_t size)
            {
            BitWriter bw;
            bw.writeBits(1, 1); // BFINAL = 1
            bw.writeBits(1, 2); // BTYPE = 01 (Fixed Huffman)

            if (size == 0)
                {
                writeFixedLitOrLen(bw, 256);
                bw.flush();
                return bw.buffer;
                }

            constexpr std::size_t kHashSize = 32768;
            int32_t head[kHashSize];
            std::fill(std::begin(head), std::end(head), -1);
            std::vector<int32_t> prev(size, -1);

            auto hash3 = [](const std::uint8_t* p) -> std::uint32_t
                {
                return ((static_cast<std::uint32_t>(p[0]) << 10) ^
                        (static_cast<std::uint32_t>(p[1]) << 5) ^
                        static_cast<std::uint32_t>(p[2])) & (kHashSize - 1);
                };

            std::size_t pos = 0;
            while (pos < size)
                {
                std::uint16_t bestLen = 0;
                std::uint16_t bestDist = 0;

                if (pos + 3 <= size)
                    {
                    const std::uint32_t h = hash3(&data[pos]);
                    int32_t cur = head[h];
                    prev[pos] = cur;
                    head[h] = static_cast<int32_t>(pos);

                    int chainLimit = 64;
                    while (cur != -1 && (pos - cur) <= 32768 && chainLimit-- > 0)
                        {
                        const std::size_t d = pos - cur;
                        const std::size_t maxLen = std::min<std::size_t>(258, size - pos);
                        std::size_t l = 0;
                        while (l < maxLen && data[pos + l] == data[cur + l])
                            {
                            ++l;
                            }
                        if (l >= 3 && l > bestLen)
                            {
                            bestLen = static_cast<std::uint16_t>(l);
                            bestDist = static_cast<std::uint16_t>(d);
                            if (bestLen == 258)
                                {
                                break;
                                }
                            }
                        cur = prev[cur];
                        }
                    }

                if (bestLen >= 3)
                    {
                    encodeLength(bw, bestLen);
                    encodeDistance(bw, bestDist);
                    for (std::size_t k = 1; k < bestLen; ++k)
                        {
                        if (pos + k + 3 <= size)
                            {
                            const std::uint32_t h = hash3(&data[pos + k]);
                            prev[pos + k] = head[h];
                            head[h] = static_cast<int32_t>(pos + k);
                            }
                        }
                    pos += bestLen;
                    }
                else
                    {
                    writeFixedLitOrLen(bw, data[pos]);
                    ++pos;
                    }
                }

            writeFixedLitOrLen(bw, 256);
            bw.flush();
            return bw.buffer;
            }

        struct CentralDirectoryEntry
            {
            std::string archivePath;
            std::uint32_t crc = 0;
            std::uint32_t compressedSize = 0;
            std::uint32_t uncompressedSize = 0;
            std::uint32_t localHeaderOffset = 0;
            std::uint16_t method = 0;
            };
        }

    core::RunResult writeZipArchive(const std::filesystem::path& zipPath,
        const std::vector<ZipEntry>& entries,
        CompressionMethod method)
        {
        std::error_code ec;
        if (zipPath.has_parent_path())
            {
            std::filesystem::create_directories(zipPath.parent_path(), ec);
            if (ec)
                {
                return { core::ExitCode::io_failure, "Failed to create directory for ZIP: " + zipPath.parent_path().string() };
                }
            }

        std::ofstream stream(zipPath, std::ios::binary);
        if (!stream)
            {
            return { core::ExitCode::io_failure, "Failed to open output ZIP file: " + zipPath.string() };
            }

        std::vector<CentralDirectoryEntry> centralDir;
        centralDir.reserve(entries.size());

        for (const auto& entry : entries)
            {
            const std::uint32_t offset = static_cast<std::uint32_t>(stream.tellp());
            const std::uint32_t uncompSize = static_cast<std::uint32_t>(entry.data.size());
            const std::uint32_t crc = calculateCrc32(entry.data.data(), entry.data.size());

            std::uint16_t entryMethod = static_cast<std::uint16_t>(CompressionMethod::store);
            std::uint32_t compSize = uncompSize;
            const std::uint8_t* dataToWrite = entry.data.data();

            std::vector<std::uint8_t> compressed;
            if (method == CompressionMethod::deflate && !entry.data.empty())
                {
                compressed = compressDeflate(entry.data.data(), entry.data.size());
                if (compressed.size() < entry.data.size())
                    {
                    entryMethod = static_cast<std::uint16_t>(CompressionMethod::deflate);
                    compSize = static_cast<std::uint32_t>(compressed.size());
                    dataToWrite = compressed.data();
                    }
                }

            char localHeader[30] = {0};
            // 0..3: Signature 0x04034b50
            localHeader[0] = 0x50;
            localHeader[1] = 0x4b;
            localHeader[2] = 0x03;
            localHeader[3] = 0x04;
            // 4..5: Version needed to extract (20 = 2.0)
            writeUInt16LE(&localHeader[4], 20);
            // 6..7: General purpose bit flag (0x0800: bit 11 set for UTF-8 path)
            writeUInt16LE(&localHeader[6], 0x0800);
            // 8..9: Compression method
            writeUInt16LE(&localHeader[8], entryMethod);
            // 10..13: Last mod file time and date (0)
            // 14..17: CRC-32
            writeUInt32LE(&localHeader[14], crc);
            // 18..21: Compressed size
            writeUInt32LE(&localHeader[18], compSize);
            // 22..25: Uncompressed size
            writeUInt32LE(&localHeader[22], uncompSize);
            // 26..27: Filename length
            const std::uint16_t nameLen = static_cast<std::uint16_t>(entry.archivePath.size());
            writeUInt16LE(&localHeader[26], nameLen);
            // 28..29: Extra field length (0)

            stream.write(localHeader, 30);
            stream.write(entry.archivePath.data(), entry.archivePath.size());
            if (compSize > 0)
                {
                stream.write(reinterpret_cast<const char*>(dataToWrite), compSize);
                }

            if (!stream)
                {
                return { core::ExitCode::io_failure, "Failed while writing ZIP entry: " + entry.archivePath };
                }

            centralDir.push_back({ entry.archivePath, crc, compSize, uncompSize, offset, entryMethod });
            }

        const std::uint32_t centralDirOffset = static_cast<std::uint32_t>(stream.tellp());

        for (const auto& cd : centralDir)
            {
            char cdHeader[46] = {0};
            // 0..3: Signature 0x02014b50
            cdHeader[0] = 0x50;
            cdHeader[1] = 0x4b;
            cdHeader[2] = 0x01;
            cdHeader[3] = 0x02;
            // 4..5: Version made by (0x0314 = Unix + PKZip 2.0)
            cdHeader[4] = 20;
            cdHeader[5] = 3;
            // 6..7: Version needed to extract (20)
            writeUInt16LE(&cdHeader[6], 20);
            // 8..9: General purpose bit flag (UTF-8)
            writeUInt16LE(&cdHeader[8], 0x0800);
            // 10..11: Compression method
            writeUInt16LE(&cdHeader[10], cd.method);
            // 12..15: Mod time and date (0)
            // 16..19: CRC-32
            writeUInt32LE(&cdHeader[16], cd.crc);
            // 20..23: Compressed size
            writeUInt32LE(&cdHeader[20], cd.compressedSize);
            // 24..27: Uncompressed size
            writeUInt32LE(&cdHeader[24], cd.uncompressedSize);
            // 28..29: Filename length
            const std::uint16_t nameLen = static_cast<std::uint16_t>(cd.archivePath.size());
            writeUInt16LE(&cdHeader[28], nameLen);
            // 30..37: Extra field len (0), comment len (0), disk start (0), internal attr (0)
            // 38..41: External file attributes (regular file 0100644 << 16)
            const std::uint32_t extAttr = (0100644U << 16);
            writeUInt32LE(&cdHeader[38], extAttr);
            // 42..45: Relative offset of local header
            writeUInt32LE(&cdHeader[42], cd.localHeaderOffset);

            stream.write(cdHeader, 46);
            stream.write(cd.archivePath.data(), cd.archivePath.size());
            }

        const std::uint32_t centralDirSize = static_cast<std::uint32_t>(stream.tellp()) - centralDirOffset;

        char eocd[22] = {0};
        // 0..3: Signature 0x06054b50
        eocd[0] = 0x50;
        eocd[1] = 0x4b;
        eocd[2] = 0x05;
        eocd[3] = 0x06;
        // 4..7: Disk number (0), disk start (0)
        // 8..9: Entries on this disk
        const std::uint16_t entryCount = static_cast<std::uint16_t>(centralDir.size());
        writeUInt16LE(&eocd[8], entryCount);
        // 10..11: Total entries
        writeUInt16LE(&eocd[10], entryCount);
        // 12..15: Size of central directory
        writeUInt32LE(&eocd[12], centralDirSize);
        // 16..19: Offset of central directory
        writeUInt32LE(&eocd[16], centralDirOffset);
        // 20..21: Comment length (0)

        stream.write(eocd, 22);

        if (!stream)
            {
            return { core::ExitCode::io_failure, "Failed to flush ZIP file: " + zipPath.string() };
            }

        return { core::ExitCode::success, "" };
        }

    core::RunResult createZipFromDirectory(const std::filesystem::path& sourceDirectory,
        const std::filesystem::path& zipPath,
        CompressionMethod method)
        {
        std::error_code ec;
        if (!std::filesystem::exists(sourceDirectory, ec) || !std::filesystem::is_directory(sourceDirectory, ec))
            {
            return { core::ExitCode::io_failure, "Source directory does not exist or is not a directory: " + sourceDirectory.string() };
            }

        std::vector<std::filesystem::path> filePaths;
        for (const auto& item : std::filesystem::recursive_directory_iterator(sourceDirectory, ec))
            {
            if (ec)
                {
                return { core::ExitCode::traversal_failure, "Failed during directory traversal: " + item.path().string() };
                }

            if (item.is_regular_file(ec))
                {
                // Skip the destination zip file if it happens to be located inside the source directory
                std::error_code eqEc;
                if (std::filesystem::exists(zipPath, eqEc) && std::filesystem::equivalent(item.path(), zipPath, eqEc))
                    {
                    continue;
                    }
                filePaths.push_back(item.path());
                }
            }

        std::sort(filePaths.begin(), filePaths.end());

        std::vector<ZipEntry> entries;
        entries.reserve(filePaths.size());

        for (const auto& filePath : filePaths)
            {
            const std::filesystem::path rel = std::filesystem::relative(filePath, sourceDirectory, ec);
            if (ec)
                {
                return { core::ExitCode::io_failure, "Failed to compute relative path for: " + filePath.string() };
                }

            std::ifstream fileStream(filePath, std::ios::binary);
            if (!fileStream)
                {
                return { core::ExitCode::io_failure, "Failed to read file: " + filePath.string() };
                }

            std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
            entries.push_back({ rel.generic_string(), std::move(data) });
            }

        return writeZipArchive(zipPath, entries, method);
        }
    }
