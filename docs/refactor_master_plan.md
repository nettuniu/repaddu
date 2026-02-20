# Refactoring Master Plan (AGENTS.md Alignment)

Objective: refactor the repository so all libraries, executables, and tests align with AGENTS.md architecture constraints without breaking behavior.
Status: completed on 2026-02-20 (all 24 tasks delivered with verification).

Scope constraints:
- Behavior compatibility is mandatory.
- No feature removals.
- Preserve output format, side effects, and error handling semantics.
- Keep dependency graph acyclic.

## Task List

### 1. Define layer/package architecture contract (completed)
Acceptance criteria:
- Add a canonical architecture doc with package/layer definitions and allowed dependencies.
- Document a strict dependency direction (`core -> io/grouping/format/ui -> cli -> app/bin`).
- Document ownership boundaries for each component.
Tests:
- `ctest --output-on-failure` (no regressions expected).
- Manual check: doc matches actual CMake target graph.

### 2. Add automated dependency-policy checker (completed)
Acceptance criteria:
- Add a script that validates:
  - only allowed target edges exist;
  - no cycles between repo targets.
- Script exits non-zero on violations and prints actionable error messages.
- Checker is runnable locally and in CI.
Tests:
- `tools/check_component_deps.sh`
- `ctest --output-on-failure`

### 3. Extract analyze-only orchestration from `app_run` (completed)
Acceptance criteria:
- Move analyze-only flow from `src/app_run.cpp` into dedicated app module.
- Keep output bytes and error paths unchanged.
- `app_run` becomes a thin orchestrator.
Tests:
- `ctest -R repaddu_test_app_run --output-on-failure`
- `ctest -R repaddu_test_analysis --output-on-failure`

### 4. Extract tag-summary rendering from app layer (completed)
Acceptance criteria:
- Move tag-summary report rendering into format/analysis-focused module.
- Keep exact output structure used by analyze-only mode.
- Keep custom tag pattern file behavior unchanged.
Tests:
- `ctest -R repaddu_test_analysis_tags_report --output-on-failure`
- `ctest -R repaddu_test_tags --output-on-failure`

### 5. Split `repaddu_core` into smaller cohesive libraries
Acceptance criteria:
- Separate foundational types/utilities from analysis-specific logic.
- Maintain zero app knowledge in base libraries.
- Link graph remains acyclic.
Tests:
- `ctest --output-on-failure`
- `tools/check_component_deps.sh`

### 6. Introduce explicit API/domain/infra folder structure
Acceptance criteria:
- Add consistent directory structure and move files incrementally.
- Public API headers stay stable or provide compatibility includes.
- Build and tests remain green.
Tests:
- `ctest --output-on-failure`

### 7. Reduce heavy includes with forward declarations
Acceptance criteria:
- Replace avoidable includes in headers with forward declarations.
- No public header includes unnecessary heavy dependencies.
- Compile succeeds on Linux/macOS/Windows CI.
Tests:
- `cmake --build build`
- `ctest --output-on-failure`

### 8. Unify error-handling conventions per module
Acceptance criteria:
- Each module follows one documented strategy (`RunResult`/exceptions where already used).
- No mixed patterns inside the same component unless justified.
Tests:
- `ctest --output-on-failure`

### 9. Refactor oversized files into focused units (`format_writer`)
Acceptance criteria:
- Further split responsibilities (planning, reading, block rendering, final write).
- Preserve exact output for markdown/jsonl/html.
Tests:
- `ctest -R repaddu_test_frontmatter_output --output-on-failure`
- `ctest -R repaddu_test_filtering --output-on-failure`

### 10. Refactor CLI parse/validation into dedicated policy units
Acceptance criteria:
- Separate parsing, validation, and defaults merge.
- Preserve all current CLI behaviors and messages.
Tests:
- `ctest -R repaddu_test_cli_parse_analysis --output-on-failure`
- `ctest -R repaddu_test_cli_config_path --output-on-failure`

### 11. Add dependency inversion for analysis backends
Acceptance criteria:
- Introduce an analysis backend interface to decouple orchestrator from concrete backend.
- Default backend behavior remains unchanged.
Tests:
- `ctest -R repaddu_test_analysis --output-on-failure`
- `ctest -R repaddu_test_analysis_lsp --output-on-failure`

### 12. Isolate filesystem operations behind narrow interfaces
Acceptance criteria:
- Introduce file-system service abstraction for traversal/read hotspots.
- Keep performance equivalent (no avoidable extra copies).
Tests:
- `ctest -R repaddu_test_filtering --output-on-failure`
- `ctest -R repaddu_test_concurrency --output-on-failure`

### 13. Stabilize configuration module boundaries
Acceptance criteria:
- Separate config load/merge/emit into distinct units.
- Keep JSON/YAML semantics and precedence unchanged.
Tests:
- `ctest -R repaddu_test_config_analysis --output-on-failure`
- `ctest -R repaddu_test_cli_config_path --output-on-failure`

### 14. Formalize immutable config snapshot at run entry
Acceptance criteria:
- Introduce immutable effective options object post-parse.
- Avoid mutable option drift during run orchestration.
Tests:
- `ctest -R repaddu_test_app_run --output-on-failure`
- `ctest -R repaddu_test_dry_run --output-on-failure`

### 15. Add architecture-focused tests for dependency constraints
Acceptance criteria:
- Add tests/commands verifying no disallowed target links.
- Ensure failures are easy to diagnose.
Tests:
- `tools/check_component_deps.sh`

### 16. Normalize naming for intent and units
Acceptance criteria:
- Rename ambiguous symbols to intent-carrying names with compatibility-preserving refactors.
- No behavior changes.
Tests:
- `ctest --output-on-failure`

### 17. Reduce lock/work scope in concurrency-sensitive paths
Acceptance criteria:
- Minimize work in synchronized sections where present.
- Preserve deterministic traversal behavior.
Tests:
- `ctest -R repaddu_test_concurrency --output-on-failure`

### 18. Improve testability of `main` and entrypoint glue
Acceptance criteria:
- Keep `main` thin and move logic into testable modules.
- Existing CLI behavior remains unchanged.
Tests:
- `ctest -R repaddu_test_app_run --output-on-failure`

### 19. Strengthen component-level test mapping (`TEST_INFO.txt`)
Acceptance criteria:
- Ensure every changed component has clear mapped tests and commands.
- Update mapping whenever files/tests move.
Tests:
- `ctest --output-on-failure`

### 20. Add performance guardrails for refactor-sensitive paths
Acceptance criteria:
- Add repeatable before/after timing command(s) for analysis/tag/read paths.
- Document acceptable variance threshold.
Tests:
- `docs/perf_baseline.md` command set

### 21. Separate UI concerns from orchestration even further
Acceptance criteria:
- Keep UI as thin adapter over UI interface; no domain logic leakage.
- CLI still works in non-rich mode by default.
Tests:
- `ctest -R repaddu_test_ui --output-on-failure`
- `ctest -R repaddu_test_app_run --output-on-failure`

### 22. Establish internal module README files per component
Acceptance criteria:
- Add short per-component readme with purpose, dependencies, and invariants.
- Keep docs concise and current.
Tests:
- Manual review + `ctest --output-on-failure`

### 23. Add migration notes for contributor-facing structure changes
Acceptance criteria:
- Update top-level docs to explain new module layout.
- Keep contributor onboarding intact.
Tests:
- `ctest --output-on-failure`

### 24. Final verification and signoff sweep
Acceptance criteria:
- Full build and test pass.
- Dependency checker passes.
- Changelog entry summarizes refactor phases.
Tests:
- `cmake --build build`
- `ctest --output-on-failure`
- `tools/check_component_deps.sh`
