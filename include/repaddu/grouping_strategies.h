#ifndef REPADDU_GROUPING_STRATEGIES_H
#define REPADDU_GROUPING_STRATEGIES_H

#include "repaddu/core_types.h"
#include "repaddu/grouping_component_map.h"

#include <optional>
#include <vector>

namespace repaddu::grouping
    {
    struct GroupingResult
        {
        std::vector<core::Group> groups;
        std::vector<std::size_t> includedIndices;
        };

    GroupingResult filterAndGroupFiles(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const ComponentMap* componentMap,
        core::RunResult& outResult);

    std::vector<core::OutputChunk> chunkGroups(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files,
        const std::vector<core::Group>& groups,
        core::RunResult& outResult);
    }

#endif // REPADDU_GROUPING_STRATEGIES_H
