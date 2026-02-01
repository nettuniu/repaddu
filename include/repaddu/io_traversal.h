#ifndef REPADDU_IO_TRAVERSAL_H
#define REPADDU_IO_TRAVERSAL_H

#include "repaddu/core_types.h"

#include <filesystem>
#include <vector>

namespace repaddu::io
    {
    struct TraversalResult
        {
        std::vector<core::FileEntry> files;
        std::vector<std::filesystem::path> directories;
        std::vector<std::filesystem::path> cmakeLists;
        };

    core::RunResult traverseRepository(const core::CliOptions& options, TraversalResult& outResult);
    }

#endif // REPADDU_IO_TRAVERSAL_H
