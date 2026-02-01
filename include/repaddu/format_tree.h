#ifndef REPADDU_FORMAT_TREE_H
#define REPADDU_FORMAT_TREE_H

#include <filesystem>
#include <string>
#include <vector>

namespace repaddu::format
    {
    std::string renderTree(const std::vector<std::filesystem::path>& directories,
        const std::vector<std::filesystem::path>& files);
    }

#endif // REPADDU_FORMAT_TREE_H
