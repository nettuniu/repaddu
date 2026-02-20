# cli

Purpose:
- Parse/bootstrap configuration and orchestrate app execution.

Primary code:
- `include/repaddu/cli_parse.h`, `src/cli_parse*.cpp`, `src/cli_bootstrap.cpp`
- `include/repaddu/cli_run.h`, `src/cli_run.cpp`
- `include/repaddu/app_run.h`, `src/app_run.cpp`
- `include/repaddu/app/*`, `src/app/*`
- `include/repaddu/config_generator.h`, `src/config_generator.cpp`

Dependencies:
- Internal: `repaddu_base`, `repaddu_analysis`, `repaddu_io`, `repaddu_grouping`, `repaddu_format`, `repaddu_ui`.

Invariants:
- Keep orchestration thin and behavior-stable.
- Respect config precedence and CLI compatibility behavior.
