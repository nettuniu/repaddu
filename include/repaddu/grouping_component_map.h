#ifndef REPADDU_GROUPING_COMPONENT_MAP_H
#define REPADDU_GROUPING_COMPONENT_MAP_H

#include "repaddu/core_types.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace repaddu::grouping
    {
    struct ComponentMap
        {
        std::unordered_map<std::string, std::vector<std::string>> componentToPrefixes;
        };

    core::RunResult loadComponentMap(const std::filesystem::path& path, ComponentMap& outMap);
    std::string resolveComponent(const ComponentMap& map, const std::filesystem::path& relativePath);
    }

#endif // REPADDU_GROUPING_COMPONENT_MAP_H
