# entrypoint

Purpose:
- Process entrypoint only.

Primary code:
- `src/main.cpp`

Dependencies:
- Internal: `repaddu_cli`.

Invariants:
- Keep `main` minimal (argument collection, parse/run handoff, exit codes).
- No domain logic in entrypoint.
