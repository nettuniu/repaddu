# REPADDU Report Card

Date: 2026-02-14
Scope: Initial execution package for `AGENT_TESTING_PLAN.md` scenarios.

## Summary

This report provides a ready-to-run matrix for evaluating LLM comprehension quality from `repaddu` outputs.
Current status is bootstrap-ready: scenarios, commands, and scoring rubric are defined.
Final accuracy scores must be filled after running the multi-agent workflow described in `AGENT_TESTING_PLAN.md`.

Canonical CLI option source: `docs/cli_spec.md`.

## Scoring Rubric

- Retrieval correctness: 0-60
- Architectural reasoning: 0-20
- Safety handling (redaction awareness): 0-10
- Hallucination control: 0-10
- Total: 0-100

## Scenario Matrix

| Scenario | Command Profile | Goal | Status | Score |
|---|---|---|---|---|
| Baseline | default | Baseline retrieval accuracy | Ready | TBD |
| Deep Context | analysis+views | Better architecture answers | Ready | TBD |
| Noise Reduction | filtering+component grouping | Lower hallucination rate | Ready | TBD |
| Safety Check | redact PII | Verify secret suppression + location awareness | Ready | TBD |
| Chunk Stress | low max-bytes | Validate cross-chunk reconstruction | Ready | TBD |

## Ready Commands

### 1. Baseline

```bash
build/repaddu --input . --output out_eval_baseline
```

### 2. Deep Context

```bash
build/repaddu --input . --output out_eval_deep --analysis --analysis-views symbols,dependencies --analysis-collapse folder
```

### 3. Noise Reduction

```bash
build/repaddu --input . --output out_eval_noise --max-file-size 10240 --group-by component --component-map components.json
```

### 4. Safety Check

```bash
build/repaddu --input . --output out_eval_safe --redact-pii
```

### 5. Chunk Stress

```bash
build/repaddu --input . --output out_eval_chunks --max-bytes 500
```

## Notes

- `components.json` is required for scenario 3.
- Fill score column only after Agent B/C/D comparison is complete.
- Keep one report revision per evaluated target repository.
- For runtime baselines, see `docs/perf_baseline.md`.
