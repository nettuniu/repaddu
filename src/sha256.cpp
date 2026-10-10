#include "repaddu/sha256.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace repaddu::crypto
    {
    namespace
        {
        constexpr std::array<std::uint32_t, 64> kK_g =
            {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
            };

        inline std::uint32_t rotr(std::uint32_t value, std::uint32_t count) noexcept
            {
            return (value >> count) | (value << (32 - count));
            }

        inline std::uint32_t ch(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept
            {
            return (x & y) ^ (~x & z);
            }

        inline std::uint32_t maj(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept
            {
            return (x & y) ^ (x & z) ^ (y & z);
            }

        inline std::uint32_t bsig0(std::uint32_t x) noexcept
            {
            return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
            }

        inline std::uint32_t bsig1(std::uint32_t x) noexcept
            {
            return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
            }

        inline std::uint32_t ssig0(std::uint32_t x) noexcept
            {
            return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
            }

        inline std::uint32_t ssig1(std::uint32_t x) noexcept
            {
            return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
            }
        }

    Sha256::Sha256()
        {
        state_[0] = 0x6a09e667;
        state_[1] = 0xbb67ae85;
        state_[2] = 0x3c6ef372;
        state_[3] = 0xa54ff53a;
        state_[4] = 0x510e527f;
        state_[5] = 0x9b05688c;
        state_[6] = 0x1f83d9ab;
        state_[7] = 0x5be0cd19;
        }

    void Sha256::transform(const std::uint8_t chunk[64])
        {
        std::array<std::uint32_t, 64> w{};
        for (std::size_t i = 0; i < 16; ++i)
            {
            w[i] = (static_cast<std::uint32_t>(chunk[i * 4]) << 24)
                 | (static_cast<std::uint32_t>(chunk[i * 4 + 1]) << 16)
                 | (static_cast<std::uint32_t>(chunk[i * 4 + 2]) << 8)
                 | static_cast<std::uint32_t>(chunk[i * 4 + 3]);
            }

        for (std::size_t i = 16; i < 64; ++i)
            {
            w[i] = ssig1(w[i - 2]) + w[i - 7] + ssig0(w[i - 15]) + w[i - 16];
            }

        std::uint32_t a = state_[0];
        std::uint32_t b = state_[1];
        std::uint32_t c = state_[2];
        std::uint32_t d = state_[3];
        std::uint32_t e = state_[4];
        std::uint32_t f = state_[5];
        std::uint32_t g = state_[6];
        std::uint32_t h = state_[7];

        for (std::size_t i = 0; i < 64; ++i)
            {
            const std::uint32_t t1 = h + bsig1(e) + ch(e, f, g) + kK_g[i] + w[i];
            const std::uint32_t t2 = bsig0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
            }

        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
        state_[4] += e;
        state_[5] += f;
        state_[6] += g;
        state_[7] += h;
        }

    void Sha256::update(std::string_view data)
        {
        const auto* bytes = reinterpret_cast<const std::uint8_t*>(data.data());
        update(std::span<const std::uint8_t>(bytes, data.size()));
        }

    void Sha256::update(std::span<const std::uint8_t> data)
        {
        totalBytes_ += data.size();
        std::size_t offset = 0;
        const std::size_t length = data.size();

        if (bufferLength_ > 0)
            {
            const std::size_t needed = 64 - bufferLength_;
            const std::size_t toCopy = std::min(needed, length);
            std::copy_n(data.data() + offset, toCopy, buffer_.data() + bufferLength_);
            bufferLength_ += toCopy;
            offset += toCopy;

            if (bufferLength_ == 64)
                {
                transform(buffer_.data());
                bufferLength_ = 0;
                }
            }

        while (offset + 64 <= length)
            {
            transform(data.data() + offset);
            offset += 64;
            }

        if (offset < length)
            {
            const std::size_t remaining = length - offset;
            std::copy_n(data.data() + offset, remaining, buffer_.data() + bufferLength_);
            bufferLength_ += remaining;
            }
        }

    std::string Sha256::finalHex()
        {
        const std::uint64_t totalBits = totalBytes_ * 8;

        // @note Append 0x80 byte
        buffer_[bufferLength_++] = 0x80;

        if (bufferLength_ > 56)
            {
            std::fill(buffer_.data() + bufferLength_, buffer_.data() + 64, static_cast<std::uint8_t>(0));
            transform(buffer_.data());
            bufferLength_ = 0;
            }

        std::fill(buffer_.data() + bufferLength_, buffer_.data() + 56, static_cast<std::uint8_t>(0));

        // @note Append 64-bit big-endian bit length
        for (int i = 7; i >= 0; --i)
            {
            buffer_[56 + (7 - i)] = static_cast<std::uint8_t>((totalBits >> (i * 8)) & 0xFF);
            }

        transform(buffer_.data());

        std::ostringstream out;
        out << std::hex << std::setfill('0');
        for (std::size_t i = 0; i < 8; ++i)
            {
            out << std::setw(8) << state_[i];
            }

        return out.str();
        }

    std::string sha256(std::string_view data)
        {
        Sha256 hasher;
        hasher.update(data);
        return hasher.finalHex();
        }

    std::string sha256(std::span<const std::uint8_t> data)
        {
        Sha256 hasher;
        hasher.update(data);
        return hasher.finalHex();
        }

    std::string sha256File(const std::filesystem::path& path)
        {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
            {
            return "";
            }

        Sha256 hasher;
        std::array<char, 65536> chunk{};
        while (file.read(chunk.data(), chunk.size()) || file.gcount() > 0)
            {
            const std::streamsize count = file.gcount();
            if (count > 0)
                {
                const auto* bytes = reinterpret_cast<const std::uint8_t*>(chunk.data());
                hasher.update(std::span<const std::uint8_t>(bytes, static_cast<std::size_t>(count)));
                }
            }

        return hasher.finalHex();
        }
    }
