#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake_file="${repo_root}/CMakeLists.txt"

if [[ ! -f "${cmake_file}" ]]; then
  echo "error: CMakeLists.txt not found at ${cmake_file}" >&2
  exit 1
fi

declare -A allowed
allowed["repaddu_io"]="repaddu_core"
allowed["repaddu_grouping"]="repaddu_core"
allowed["repaddu_format"]="repaddu_core"
allowed["repaddu_cli"]="repaddu_core repaddu_io repaddu_grouping repaddu_format repaddu_ui"
allowed["repaddu_cpp_analyzer"]="repaddu_core"
allowed["repaddu"]="repaddu_cli"

edges_file="$(mktemp)"
cleanup()
  {
  rm -f "${edges_file}"
  }
trap cleanup EXIT

awk '
  BEGIN { in_block=0; target=""; }
  /^[[:space:]]*target_link_libraries[[:space:]]*\(/ {
    in_block=1;
    line=$0;
    sub(/^[[:space:]]*target_link_libraries[[:space:]]*\(/, "", line);
    gsub(/[[:space:]]+/, " ", line);
    split(line, p, " ");
    target=p[1];
    next;
  }
  in_block {
    if ($0 ~ /\)/) {
      in_block=0;
      target="";
      next;
    }
    line=$0;
    gsub(/#.*/, "", line);
    gsub(/[[:space:]]+/, " ", line);
    split(line, p, " ");
    for (i in p) {
      dep=p[i];
      if (dep == "" || dep == "PUBLIC" || dep == "PRIVATE" || dep == "INTERFACE") {
        continue;
      }
      if (target ~ /^repaddu/ && dep ~ /^repaddu/ && target !~ /^repaddu_test_/) {
        print target " " dep;
      }
    }
  }
' "${cmake_file}" | sort -u > "${edges_file}"

status=0

while read -r from to; do
  [[ -z "${from}" || -z "${to}" ]] && continue
  allowed_list="${allowed[${from}]:-}"
  if [[ -z "${allowed_list}" ]]; then
    echo "dependency policy violation: source target '${from}' is not in allowed map" >&2
    status=1
    continue
  fi
  is_allowed=1
  for allowed_dep in ${allowed_list}; do
    if [[ "${to}" == "${allowed_dep}" ]]; then
      is_allowed=0
      break
    fi
  done
  if [[ ${is_allowed} -ne 0 ]]; then
    echo "dependency policy violation: disallowed edge '${from} -> ${to}'" >&2
    status=1
  fi
done < "${edges_file}"

if command -v tsort >/dev/null 2>&1; then
  if ! tsort "${edges_file}" >/dev/null 2>&1; then
    echo "dependency policy violation: cycle detected in target graph" >&2
    status=1
  fi
else
  echo "warning: tsort not found, cycle check skipped" >&2
fi

if [[ ${status} -ne 0 ]]; then
  exit ${status}
fi

echo "Dependency policy check passed."
