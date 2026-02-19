# Error Handling Contract

This document defines the module-level error-handling strategy and keeps behavior stable during refactoring.

## Primary Rule

- Runtime failures are propagated with `core::RunResult` across module boundaries.
- `ExitCode` values and messages are part of observable behavior and must remain stable unless explicitly changed.

## Module Strategies

- `cli` / `app` / `io` / `grouping` / `format` / `analysis`:
  - Public operations return `core::RunResult` (or structs that contain it).
  - Internal helper functions may use `bool` for local parse checks.
  - Exceptions must not escape module public APIs.

- `json_lite`:
  - Parsing returns `core::RunResult`.
  - Internal conversion helpers may use local exception guards, but parse API remains `RunResult`-based.

## Refactoring Constraints

- Do not mix exception propagation and `RunResult` for public entry points in the same module.
- Keep error ordering and early-return behavior unchanged where observable.
- Keep invalid-usage errors deterministic for CLI parsing.
