#ifndef REPADDU_CLI_RUN_H
#define REPADDU_CLI_RUN_H

#include "repaddu/core_types.h"
#include "repaddu/ui_interface.h"

namespace repaddu::cli
    {
    core::RunResult run(const core::CliOptions& options, ui::UserInterface* ui = nullptr);
    }

#endif // REPADDU_CLI_RUN_H
