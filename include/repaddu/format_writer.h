#ifndef REPADDU_FORMAT_WRITER_H
#define REPADDU_FORMAT_WRITER_H

#include "repaddu/core_types.h"

#include <filesystem>
#include <string>
#include <vector>

namespace repaddu::format
    {
    core::RunResult writeOutputs(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const std::vector<core::OutputChunk>& chunks,
        const std::string& treeListing,
        const std::vector<std::filesystem::path>& cmakeLists,
        const std::vector<std::filesystem::path>& buildFiles);
    }

#endif // REPADDU_FORMAT_WRITER_H
