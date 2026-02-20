# ui

Purpose:
- User-interface adapters implementing the `UserInterface` contract.

Primary code:
- `include/repaddu/ui_interface.h`
- `include/repaddu/ui_console.h`, `src/ui_console.cpp`
- `include/repaddu/ui_plain.h`, `src/ui_plain.cpp`

Dependencies:
- Internal: none required by contract.
- External: standard I/O streams.

Invariants:
- Keep UI adapter-only: formatting/progress concerns only, no domain orchestration logic.
- Default CLI behavior should remain non-rich unless an explicit richer UI is provided.
