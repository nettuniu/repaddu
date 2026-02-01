#ifndef REPADDU_FORMAT_LANGUAGE_REPORT_H
#define REPADDU_FORMAT_LANGUAGE_REPORT_H

#include "repaddu/core_types.h"

#include <string>
#include <vector>

namespace repaddu::format
    {
    std::string renderLanguageReport(const core::CliOptions& options,
        const std::vector<core::FileEntry>& files);
    }

#endif // REPADDU_FORMAT_LANGUAGE_REPORT_H
