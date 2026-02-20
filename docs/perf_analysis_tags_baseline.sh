#!/usr/bin/env bash
set -euo pipefail

BIN=${1:-build/repaddu}
INPUT=${2:-.}
OUT=${3:-/tmp/repaddu_perf_out}
LOOPS=${4:-20}
READ_INPUT=${5:-fixtures/sample_repo}

echo "Running baseline with bin=$BIN input=$INPUT read_input=$READ_INPUT loops=$LOOPS"

/usr/bin/time -f "analyze_only_${LOOPS}x_seconds=%e" bash -lc "for i in \$(seq 1 ${LOOPS}); do ${BIN} --input ${INPUT} --output ${OUT} --analyze-only >/dev/null 2>&1; done" 2>&1 | tail -n 1
/usr/bin/time -f "analyze_tags_${LOOPS}x_seconds=%e" bash -lc "for i in \$(seq 1 ${LOOPS}); do ${BIN} --input ${INPUT} --output ${OUT} --analyze-only --extract-tags >/dev/null 2>&1; done" 2>&1 | tail -n 1
/usr/bin/time -f "read_emit_${LOOPS}x_seconds=%e" bash -lc "for i in \$(seq 1 ${LOOPS}); do ${BIN} --input ${READ_INPUT} --output ${OUT} --group-by directory --group-depth 2 --max-bytes 200000 >/dev/null 2>&1; done" 2>&1 | tail -n 1
