#!/usr/bin/env bash
set -euo pipefail

BIN=${1:-build/repaddu}
INPUT=${2:-.}
OUT=${3:-/tmp/repaddu_perf_guard_out}
LOOPS=${4:-20}
READ_INPUT=${5:-fixtures/sample_repo}

# Thresholds are totals over LOOPS runs.
ANALYZE_ONLY_MAX=${ANALYZE_ONLY_MAX:-0.50}
ANALYZE_TAGS_MAX=${ANALYZE_TAGS_MAX:-0.55}
READ_EMIT_MAX=${READ_EMIT_MAX:-0.35}

run_total_seconds()
{
  local label=$1
  local cmd=$2
  local max_allowed=$3
  local value

  value=$(/usr/bin/time -f '%e' bash -lc "for i in \$(seq 1 ${LOOPS}); do ${cmd} >/dev/null 2>&1; done" 2>&1 | tail -n 1)

  echo "${label}_total_seconds=${value} (max=${max_allowed})"

  awk -v actual="${value}" -v threshold="${max_allowed}" 'BEGIN { exit !(actual <= threshold) }'
}

echo "Performance guard: bin=${BIN} input=${INPUT} read_input=${READ_INPUT} loops=${LOOPS}"

run_total_seconds "analyze_only" "${BIN} --input ${INPUT} --output ${OUT} --analyze-only" "${ANALYZE_ONLY_MAX}"
run_total_seconds "analyze_tags" "${BIN} --input ${INPUT} --output ${OUT} --analyze-only --extract-tags" "${ANALYZE_TAGS_MAX}"
run_total_seconds "read_emit" "${BIN} --input ${READ_INPUT} --output ${OUT} --group-by directory --group-depth 2 --max-bytes 200000" "${READ_EMIT_MAX}"

echo "Performance guard passed."
