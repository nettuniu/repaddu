#include "repaddu/cli_run.h"

#include "repaddu/app_run.h"
#include "repaddu/ui_plain.h"

namespace repaddu::cli
    {
    core::RunResult run(const core::CliOptions& options, ui::UserInterface* ui)
        {
        ui::PlainUI defaultUI;
        if (!ui)
            {
            ui = &defaultUI;
            }

        return app::run(options, *ui);
        }
    }
