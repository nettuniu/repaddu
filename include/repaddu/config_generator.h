#ifndef REPADDU_CONFIG_GENERATOR_H
#define REPADDU_CONFIG_GENERATOR_H

#include "repaddu/core_types.h"

namespace repaddu::config
    {
    core::RunResult generateDefaultConfig(const std::filesystem::path& path = ".repaddu.json");
    }

#endif // REPADDU_CONFIG_GENERATOR_H
