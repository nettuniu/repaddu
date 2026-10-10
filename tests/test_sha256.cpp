#include "repaddu/sha256.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
    {
    void test_sha256_known_vectors()
        {
        // Vector 1: Empty string
        const std::string hashEmpty = repaddu::crypto::sha256("");
        assert(hashEmpty == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

        // Vector 2: "abc"
        const std::string hashAbc = repaddu::crypto::sha256("abc");
        assert(hashAbc == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

        // Vector 3: "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
        const std::string text3 = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
        const std::string hash3 = repaddu::crypto::sha256(text3);
        assert(hash3 == "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
        }

    void test_sha256_incremental_updates()
        {
        repaddu::crypto::Sha256 hasher;
        hasher.update("a");
        hasher.update("b");
        hasher.update("c");
        const std::string res = hasher.finalHex();
        assert(res == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
        }

    void test_sha256_file_hashing()
        {
        const std::filesystem::path tempFile = std::filesystem::temp_directory_path() / "repaddu_test_sha.txt";
        {
            std::ofstream out(tempFile, std::ios::binary);
            out << "abc";
        }

        const std::string fileHash = repaddu::crypto::sha256File(tempFile);
        assert(fileHash == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

        std::error_code ec;
        std::filesystem::remove(tempFile, ec);
        }
    }

int main()
    {
    test_sha256_known_vectors();
    test_sha256_incremental_updates();
    test_sha256_file_hashing();
    std::cout << "SHA-256 tests passed successfully.\n";
    return 0;
    }
