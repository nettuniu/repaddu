#include "repaddu/io_binary.h"
#include <fstream>
#include <cassert>
#include <filesystem>
#include <vector>
#include <iostream>

namespace fs = std::filesystem;

void create_file(const std::string& path, const std::vector<uint8_t>& content)
    {
    std::ofstream ofs(path, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(content.data()), content.size());
    }

void test_png_detection()
    {
    std::string path = "test_image.png";
    // PNG signature: 89 50 4E 47 0D 0A 1A 0A
    std::vector<uint8_t> content = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x01};
    create_file(path, content);
    
    assert(repaddu::io::looksBinary(path) == true);
    fs::remove(path);
    std::cout << "PNG detection passed." << std::endl;
    }

void test_text_detection()
    {
    std::string path = "test_text.txt";
    std::string text = "Hello world\nThis is a text file.";
    std::vector<uint8_t> content(text.begin(), text.end());
    create_file(path, content);

    assert(repaddu::io::looksBinary(path) == false);
    fs::remove(path);
    std::cout << "Text detection passed." << std::endl;
    }

void test_nul_detection()
    {
    std::string path = "test_nul.bin";
    std::vector<uint8_t> content = {'H', 'e', 'l', 'l', 'o', 0x00, 'W', 'o', 'r', 'l', 'd'};
    create_file(path, content);

    assert(repaddu::io::looksBinary(path) == true);
    fs::remove(path);
    std::cout << "NUL detection passed." << std::endl;
    }

int main()
    {
    test_png_detection();
    test_text_detection();
    test_nul_detection();
    std::cout << "All binary detection tests passed!" << std::endl;
    return 0;
    }
