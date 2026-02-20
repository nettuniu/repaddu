# repaddu

Convert a repository/folder into numbered Markdown files for LLM/RAG ingestion.

## Documentation

- Canonical CLI option reference: `docs/cli_spec.md`
- Full user/contributor manual: `docs/GUIDE.md`
- Architecture dependency contract: `docs/architecture_layers.md`
- Error handling contract: `docs/error_handling_contract.md`
- Refactor master plan: `docs/refactor_master_plan.md`

## Contributor migration notes (module layout)

`repaddu` has moved from a mostly flat/core-heavy structure to layered modules.
When touching older code, use this mapping:

- Foundational and shared domain types/utilities: `repaddu_base`
- Analysis domain logic (graph/view/LSP): `repaddu_analysis`
- Infrastructure adapters:
  - Filesystem/binary detection: `repaddu_io`
  - Grouping/filter planning: `repaddu_grouping`
  - Output/report formatting: `repaddu_format`
  - Console/UI adapters: `repaddu_ui`
- Application orchestration and CLI policy: `repaddu_cli`
- Entrypoint executable: `src/main.cpp` (thin handoff to CLI layer)

Current compatibility targets and wrappers remain in place during migration:
- `repaddu_core` is a compatibility aggregate target.
- Compatibility headers under `include/repaddu/*.h` are preserved while files are
  incrementally relocated (for example `include/repaddu/app/*`,
  `include/repaddu/format/*`).

Before and after moving code, validate affected test scopes from `TEST_INFO.txt`.

## Quick start

```bash
repaddu --input . --output ./out
```

## Common usage

```bash
# Group by file type and list headers before sources
repaddu -i . -o out --group-by type --headers-first

# Scan repository language mix (no output files)
repaddu -i . --scan-languages

# Limit output to 12 files with a 200kB cap per file
repaddu -i . -o out --max-files 12 --max-bytes 200000

# Include only headers and sources explicitly
repaddu -i . -o out --extensions h,hpp,cpp

# Group by directory depth 2
repaddu -i . -o out --group-by directory --group-depth 2

# Size-balanced grouping with a capacity limit
repaddu -i . -o out --group-by size --max-bytes 200000

# Component map grouping (JSON)
repaddu -i . -o out --group-by component --component-map components.json

# Aggregate build-system files for a Rust repo
repaddu -i . -o out --language rust --emit-build-files
```

## Component map format

The component map is a JSON object where each key is a component name and the value
is an array of relative path prefixes.

```json
{
  "core": ["src/core", "include/core"],
  "cli": ["src/cli", "include/cli"]
}
```

## Output overview

- `000_overview.md` documents the boundary markers and file layout.
- Other output files begin with a reminder to read the overview first.
