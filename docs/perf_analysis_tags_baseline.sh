#!/usr/bin/env bash
set -euo pipefail

BIN=${1:-build/repaddu}
INPUT=${2:-.}
OUT=${3:-/tmp/repaddu_perf_out}
LOOPS=${4:-20}

echo "Running baseline with bin=$BIN input=$INPUT loops=$LOOPS"

/usr/bin/time -f 'analyze_only_20x_seconds=%e' bash -lc "for i in \$(seq 1 ${LOOPS}); do ${BIN} --input ${INPUT} --output ${OUT} --analyze-only >/dev/null 2>&1; done" 2>&1 | tail -n 1
/usr/bin/time -f 'analyze_tags_20x_seconds=%e' bash -lc "for i in \$(seq 1 ${LOOPS}); do ${BIN} --input ${INPUT} --output ${OUT} --analyze-only --extract-tags >/dev/null 2>&1; done" 2>&1 | tail -n 1
/usr/bin/time -f 'dry_run_20x_seconds=%e' bash -lc "for i in \$(seq 1 ${LOOPS}); do ${BIN} --input ${INPUT} --output ${OUT} --dry-run >/dev/null 2>&1; done" 2>&1 | tail -n 1
