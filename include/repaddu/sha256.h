#ifndef REPADDU_SHA256_H
#define REPADDU_SHA256_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace repaddu::crypto
    {
    class Sha256
        {
        public:
            Sha256();
            void update(std::string_view data);
            void update(std::span<const std::uint8_t> data);
            std::string finalHex();

        private:
            void transform(const std::uint8_t chunk[64]);
            std::array<std::uint32_t, 8> state_{};
            std::array<std::uint8_t, 64> buffer_{};
            std::size_t bufferLength_ = 0;
            std::uint64_t totalBytes_ = 0;
        };

    // @note Convenience hashing helpers
    std::string sha256(std::string_view data);
    std::string sha256(std::span<const std::uint8_t> data);
    std::string sha256File(const std::filesystem::path& path);
    }

#endif // REPADDU_SHA256_H
