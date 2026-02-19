#include "format_writer_alt_formats.h"

#include "repaddu/analysis_tokens.h"

#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace repaddu::format::detail
    {
    namespace
        {
        bool tryReadFileMmap(const std::filesystem::path& path, std::string& outContent)
            {
#if defined(_WIN32)
            const std::wstring widePath = path.wstring();
            HANDLE fileHandle = CreateFileW(widePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (fileHandle == INVALID_HANDLE_VALUE)
                {
                return false;
                }

            LARGE_INTEGER size;
            if (!GetFileSizeEx(fileHandle, &size))
                {
                CloseHandle(fileHandle);
                return false;
                }

            if (size.QuadPart <= 0)
                {
                outContent.clear();
                CloseHandle(fileHandle);
                return true;
                }

            if (size.QuadPart > static_cast<LONGLONG>((std::numeric_limits<std::size_t>::max)()))
                {
                CloseHandle(fileHandle);
                return false;
                }

            HANDLE mapping = CreateFileMappingW(fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
            if (!mapping)
                {
                CloseHandle(fileHandle);
                return false;
                }

            void* view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
            if (!view)
                {
                CloseHandle(mapping);
                CloseHandle(fileHandle);
                return false;
                }

            const std::size_t sizeBytes = static_cast<std::size_t>(size.QuadPart);
            const char* data = static_cast<const char*>(view);
            outContent.assign(data, data + sizeBytes);

            UnmapViewOfFile(view);
            CloseHandle(mapping);
            CloseHandle(fileHandle);
            return true;
#else
            const int fd = ::open(path.c_str(), O_RDONLY);
            if (fd < 0)
                {
                return false;
                }

            struct stat statbuf;
            if (::fstat(fd, &statbuf) != 0)
                {
                ::close(fd);
                return false;
                }

            if (statbuf.st_size <= 0)
                {
                outContent.clear();
                ::close(fd);
                return true;
                }

            if (static_cast<unsigned long long>(statbuf.st_size) > std::numeric_limits<std::size_t>::max())
                {
                ::close(fd);
                return false;
                }

            void* mapping = ::mmap(nullptr, static_cast<std::size_t>(statbuf.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
            if (mapping == MAP_FAILED)
                {
                ::close(fd);
                return false;
                }

            const std::size_t sizeBytes = static_cast<std::size_t>(statbuf.st_size);
            const char* data = static_cast<const char*>(mapping);
            outContent.assign(data, data + sizeBytes);

            ::munmap(mapping, sizeBytes);
            ::close(fd);
            return true;
#endif
            }
        }

    std::string readFileContent(const std::filesystem::path& path,
        core::RunResult& outResult,
        std::uintmax_t* outTokens,
        security::PiiRedactor* redactor,
        const std::string& relativePath)
        {
        std::string content;
        if (!tryReadFileMmap(path, content))
            {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
                {
                outResult = { core::ExitCode::io_failure, "Failed to open file for reading." };
                return {};
                }
            std::ostringstream buffer;
            buffer << stream.rdbuf();
            content = buffer.str();
            }

        outResult = { core::ExitCode::success, "" };

        if (redactor)
            {
            content = redactor->redact(content, relativePath);
            }

        if (outTokens)
            {
            *outTokens = analysis::TokenEstimator::estimateTokens(content);
            }

        return content;
        }
    }
