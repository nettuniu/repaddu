#ifndef REPADDU_APP_EFFECTIVE_OPTIONS_H
#define REPADDU_APP_EFFECTIVE_OPTIONS_H

#include "repaddu/core_types.h"

#include <vector>

namespace repaddu::app
    {
    core::CliOptions buildEffectiveOptions(const core::CliOptions& requestedOptions,
        const std::vector<core::FileEntry>& files);
    }

#endif // REPADDU_APP_EFFECTIVE_OPTIONS_H
