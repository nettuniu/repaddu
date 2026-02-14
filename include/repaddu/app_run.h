#ifndef REPADDU_APP_RUN_H
#define REPADDU_APP_RUN_H

#include "repaddu/core_types.h"
#include "repaddu/ui_interface.h"

namespace repaddu::app
    {
    core::RunResult run(const core::CliOptions& options, ui::UserInterface& ui);
    }

#endif // REPADDU_APP_RUN_H
