# Architecture Layers and Dependency Contract

This document is the canonical dependency contract for repository components.

## Layer Model

1. `base/core`:
   - Contains foundational types, parsing-independent utilities, analysis models/utilities.
   - Must not depend on app orchestration or UI implementation details.
2. `infra`:
   - `io`, `grouping`, `format`, `ui`.
   - May depend on `core`.
   - Must not depend on `cli`/application orchestration.
3. `application`:
   - `cli` orchestration modules and run flow.
   - May depend on `core` and infra libraries.
4. `entrypoint`:
   - `repaddu` executable only.
   - Delegates to application layer (`cli`).

## Target Dependency Policy

Allowed direct internal edges:

- `repaddu_io -> repaddu_core`
- `repaddu_grouping -> repaddu_core`
- `repaddu_format -> repaddu_core`
- `repaddu_cli -> repaddu_core`
- `repaddu_cli -> repaddu_io`
- `repaddu_cli -> repaddu_grouping`
- `repaddu_cli -> repaddu_format`
- `repaddu_cli -> repaddu_ui`
- `repaddu_cpp_analyzer -> repaddu_core` (optional target)
- `repaddu -> repaddu_cli`

Disallowed examples:

- `repaddu_core -> repaddu_*` (other than itself)
- `repaddu_format -> repaddu_cli`
- `repaddu_io -> repaddu_grouping`
- Any cycle across repo targets

## Component Ownership

- `core`: `core_types`, `language_profiles`, logger/redaction, token/tag extraction, analysis graph/view/lsp, lightweight json parser.
- `io`: filesystem traversal and binary detection.
- `grouping`: filtering/grouping/chunk planning inputs.
- `format`: markdown/jsonl/html writers and analysis rendering.
- `ui`: user-interface abstraction and console implementation.
- `cli/app`: argument parse/config load/generation and app orchestration.

## Verification

Run dependency policy checker:

```bash
tools/check_component_deps.sh
```

Run full tests:

```bash
ctest --test-dir build --output-on-failure
```
