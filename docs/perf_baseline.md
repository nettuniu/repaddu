# Performance Baseline

Date: 2026-02-14
Machine: local dev environment
Target binary: `build/repaddu`
Inputs:
- analysis/tag profiles: repository root (`--input .`)
- read/write profile: `fixtures/sample_repo`

## Before/After Command Set

Capture baseline (before change):

```bash
./docs/perf_analysis_tags_baseline.sh build/repaddu . /tmp/repaddu_perf_out 20 fixtures/sample_repo
```

Capture candidate (after change):

```bash
./docs/perf_analysis_tags_baseline.sh build/repaddu . /tmp/repaddu_perf_out_after 20 fixtures/sample_repo
```

Automated guard (same three paths):

```bash
./docs/perf_regression_guard.sh build/repaddu . /tmp/repaddu_perf_guard_out 20 fixtures/sample_repo
```

## Baseline Results (20 runs each)

- `analyze_only_20x_seconds=0.24`
- `analyze_tags_20x_seconds=0.25`
- `read_emit_20x_seconds=0.19`

## Acceptable Variance Threshold

- Preferred guardrail for refactor-sensitive paths: `<= 15%` slowdown per profile
  when comparing `before` and `after` runs on the same machine and load state.
- For quick CI/local gate checks, absolute totals are enforced by
  `docs/perf_regression_guard.sh` defaults:
  - `ANALYZE_ONLY_MAX=0.50`
  - `ANALYZE_TAGS_MAX=0.55`
  - `READ_EMIT_MAX=0.35`

## Notes

- These are wall-clock totals for 20 invocations per profile.
- Use this file as the pre-change reference for follow-up performance work.
- `read_emit` intentionally exercises the read + grouping + emit path instead of
  dry-run-only behavior.
- CLI flag semantics are defined in `docs/cli_spec.md`.
