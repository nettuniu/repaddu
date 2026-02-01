# repaddu

Convert a repository/folder into numbered Markdown files for LLM/RAG ingestion.

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
