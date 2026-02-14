#ifndef REPADDU_FORMAT_WRITER_ALT_FORMATS_H
#define REPADDU_FORMAT_WRITER_ALT_FORMATS_H

#include "repaddu/core_types.h"
#include "repaddu/pii_redactor.h"

#include <filesystem>
#include <string>
#include <vector>

namespace repaddu::format::detail
    {
    std::string readFileContent(const std::filesystem::path& path,
        core::RunResult& outResult,
        std::uintmax_t* outTokens = nullptr,
        security::PiiRedactor* redactor = nullptr,
        const std::string& relativePath = "");

    core::RunResult writeJsonlOutput(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const std::vector<core::OutputChunk>& chunks,
        security::PiiRedactor* redactor);

    core::RunResult writeHtmlOutput(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        security::PiiRedactor* redactor);
    }

#endif // REPADDU_FORMAT_WRITER_ALT_FORMATS_H
