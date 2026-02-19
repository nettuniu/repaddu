# Architecture Layers and Dependency Contract

This document is the canonical dependency contract for repository components.

## Layer Model

1. `base`:
   - Contains foundational types, parsing-independent utilities, analysis models/utilities.
   - Must not depend on app orchestration or UI implementation details.
2. `analysis`:
   - Contains graph/view/LSP analysis logic.
   - May depend on `base`.
2. `infra`:
   - `io`, `grouping`, `format`, `ui`.
   - May depend on `base` and `analysis` where required.
   - Must not depend on `cli`/application orchestration.
3. `application`:
   - `cli` orchestration modules and run flow.
   - May depend on `base`, `analysis`, and infra libraries.
4. `entrypoint`:
   - `repaddu` executable only.
   - Delegates to application layer (`cli`).

## Target Dependency Policy

Allowed direct internal edges:

- `repaddu_analysis -> repaddu_base`
- `repaddu_core -> repaddu_base` (compatibility aggregate)
- `repaddu_core -> repaddu_analysis` (compatibility aggregate)
- `repaddu_io -> repaddu_base`
- `repaddu_grouping -> repaddu_base`
- `repaddu_format -> repaddu_base`
- `repaddu_format -> repaddu_analysis`
- `repaddu_cli -> repaddu_base`
- `repaddu_cli -> repaddu_analysis`
- `repaddu_cli -> repaddu_io`
- `repaddu_cli -> repaddu_grouping`
- `repaddu_cli -> repaddu_format`
- `repaddu_cli -> repaddu_ui`
- `repaddu_cpp_analyzer -> repaddu_analysis` (optional target)
- `repaddu -> repaddu_cli`

Disallowed examples:

- `repaddu_base -> repaddu_*` (other than itself)
- `repaddu_analysis -> repaddu_*` except `repaddu_base`
- `repaddu_format -> repaddu_cli`
- `repaddu_io -> repaddu_grouping`
- Any cycle across repo targets

## Component Ownership

- `base`: `core_types`, `language_profiles`, logger/redaction, token/tag extraction, lightweight json parser.
- `analysis`: analysis graph/view/lsp.
- `io`: filesystem traversal and binary detection.
- `grouping`: filtering/grouping/chunk planning inputs.
- `format`: markdown/jsonl/html writers and analysis rendering.
- `ui`: user-interface abstraction and console implementation.
- `cli/app`: argument parse/config load/generation and app orchestration.
  - `src/cli_bootstrap.cpp` is the boundary for config resolution+load before parse.
  - `src/app/effective_options.cpp` builds immutable run-time options snapshot.

## Incremental Physical Structure

Refactor work introduces explicit subfolders incrementally while preserving compatibility include paths:

- Application: `include/repaddu/app/*`, `src/app/*`
- Infrastructure format: `include/repaddu/format/*`, `src/format/*`

Compatibility wrappers under `include/repaddu/*.h` are kept during migration.

## Verification

Run dependency policy checker:

```bash
tools/check_component_deps.sh
```

Run full tests:

```bash
ctest --test-dir build --output-on-failure
```
