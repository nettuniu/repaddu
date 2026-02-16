#!/usr/bin/env bash
set -euo pipefail

BIN=${1:-build/repaddu}
INPUT=${2:-.}
OUTROOT=${3:-/tmp/repaddu_eval_smoke}
mkdir -p "$OUTROOT"

COMPONENT_MAP="$OUTROOT/components.json"
cat > "$COMPONENT_MAP" <<'JSON'
{
  "src": ["src"],
  "include": ["include"],
  "tests": ["tests"],
  "docs": ["docs"],
  "fixtures": ["fixtures"]
}
JSON

run_case()
{
  local name="$1"
  shift
  local log="$OUTROOT/${name}.log"
  local outDir="$OUTROOT/out_eval_${name}"
  local t0 t1 ms code

  t0=$(date +%s%3N)
  set +e
  "$BIN" --input "$INPUT" --output "$outDir" "$@" >"$log" 2>&1
  code=$?
  set -e
  t1=$(date +%s%3N)
  ms=$((t1 - t0))

  local fileCount=0
  if [ -d "$outDir" ]; then
    fileCount=$(find "$outDir" -maxdepth 1 -type f | wc -l | tr -d ' ')
  fi

  echo "${name}|exit=${code}|time_ms=${ms}|files=${fileCount}|log=${log}"
}

echo "Running eval smoke matrix with bin=${BIN} input=${INPUT} out=${OUTROOT}"
run_case baseline
run_case deep --analysis --analysis-views symbols,dependencies --analysis-collapse folder
run_case noise --max-file-size 10240 --group-by component --component-map "$COMPONENT_MAP"
run_case safe --redact-pii
run_case chunks --max-bytes 500

echo "Done."
