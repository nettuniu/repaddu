# Performance Baseline

Date: 2026-02-14
Machine: local dev environment
Target binary: `build/repaddu`
Input: repository root (`--input .`)

## Repro Command

```bash
./docs/perf_analysis_tags_baseline.sh build/repaddu . /tmp/repaddu_perf_out 20
```

## Baseline Results (20 runs each)

- `analyze_only_20x_seconds=0.24`
- `analyze_tags_20x_seconds=0.25`
- `dry_run_20x_seconds=0.26`

## Notes

- These are wall-clock totals for 20 invocations per profile.
- Use this file as the pre-change reference for follow-up performance work.
- CLI flag semantics are defined in `docs/cli_spec.md`.
