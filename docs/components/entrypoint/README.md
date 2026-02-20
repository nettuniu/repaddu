# entrypoint

Purpose:
- Process entrypoint only.

Primary code:
- `include/repaddu/entrypoint_main.h`, `src/entrypoint_main.cpp`
- `src/main.cpp`

Dependencies:
- Internal: `repaddu_cli`.

Invariants:
- Keep `main` minimal (argument collection + delegation only).
- Keep parse/run/exit behavior centralized in `entrypoint_main`.
- No domain logic in entrypoint.
