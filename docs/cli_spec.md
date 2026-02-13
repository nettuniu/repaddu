# CLI Specification

## Overview
Command name: `repaddu`

Purpose: Convert a repository/folder into a sequence of Markdown files for LLM/RAG ingestion.

## Syntax
```
repaddu [options] --input <path> --output <path>
```

## Options
### Required
- `-i, --input <path>`
  - Input repository or folder path.
- `-o, --output <path>`
  - Output directory where numbered Markdown files are written.
  - Optional when using `--scan-languages`.

### Output sizing and numbering
- `--max-files <count>`
  - Maximum number of output files to generate.
  - Default: `0` (no limit; use size or grouping rules).
- `--max-bytes <bytes>`
  - Maximum bytes per output file (content bytes, not filesystem size).
  - Default: `0` (no limit).
- `--number-width <n>`
  - Width of numeric prefix (e.g., `3` → `000`).
  - Default: `3`.

### Filtering
- `--include-headers`
  - Include header files only (`.h`, `.hpp`, `.hh`, `.hxx`).
  - Default: `false`.
- `--include-sources`
  - Include source files (`.c`, `.cc`, `.cpp`, `.cxx`).
  - Default: `true` unless `--include-headers` is used without this flag.
- `--extensions <csv>`
  - Comma-separated list of extensions to include (overrides include-headers/include-sources).
  - Default: empty (derived from include-headers/include-sources).
- `--exclude-extensions <csv>`
  - Comma-separated list of extensions to exclude (applied after includes).
  - Default: empty.
- `--include-hidden`
  - Include hidden files and folders (those beginning with a dot).
  - Default: `false`.
- `--follow-symlinks`
  - Follow directory symlinks.
  - Default: `false`.
- `--single-thread`
  - Force single-threaded traversal.
  - Default: `false`.
- `--parallel-traversal`
  - Enable parallel traversal.
  - Default: `true`.

### Binary handling
- `--include-binaries`
  - Include binary files (normally excluded).
  - Default: `false`.

### Grouping strategies
- `--group-by <mode>`
  - Grouping strategy for output splitting.
  - Allowed values: `directory`, `component`, `type`, `size`.
  - Default: `directory`.
- `--group-depth <n>`
  - Directory depth used when `--group-by directory`.
  - Default: `1`.
- `--component-map <path>`
  - Mapping file for `--group-by component`.
  - Format: JSON object mapping component names to arrays of path prefixes.
  - Default: empty (required when using `component`).
- `--headers-first`
  - When grouping allows ordering, output headers before sources.
  - Default: `false`.

### Output content options
- `--emit-tree`
  - Emit a recursive ls-like listing section.
  - Default: `true`.
- `--emit-cmake`
  - Aggregate all `CMakeLists.txt` into a dedicated output file.
  - Default: `true`.
- `--emit-build-files`
  - Aggregate build-system files (based on selected language/build system).
  - Default: `false`.
- `--markers <mode>`
  - Boundary marker style for file content blocks.
  - Allowed values: `fenced`, `sentinel`.
  - Default: `fenced`.
- `--frontmatter`
  - Add YAML frontmatter metadata before each emitted file content block.
  - Default: `false`.
- `--no-links`
  - Disable markdown links in overview table of contents (plain filenames only).
  - Default: links enabled.
- `--extract-tags`
  - Include TODO/FIXME-like tag summary in `--analyze-only` textual output.
  - Default: `false`.
- `--tag-patterns <path>`
  - Load additional tag patterns from file (one pattern per line, `#` comments allowed).
  - Default: empty.
### Language and build system
- `--language <id>`
  - Explicit language profile: `auto`, `c`, `cpp`, `rust`, `python`.
  - Default: `auto` (current C/C++ behavior).
- `--build-system <id>`
  - Explicit build system profile: `auto`, `cmake`, `make`, `meson`, `bazel`, `cargo`, `python`.
  - Default: `auto`.

### Misc
- `--config <path>`
  - Config file path to load and/or generate.
  - Default: `.repaddu.json`.
- `-h, --help`
  - Show help and exit with code 0.
- `--version`
  - Show version and exit with code 0.
- `--scan-languages`
  - Scan repository and report language percentages only (no output files).

## Precedence and conflicts
- Config precedence: defaults < config file (`--config` path or `.repaddu.json`) < CLI flags.
- `--extensions` overrides `--include-headers` and `--include-sources`.
- If both `--include-headers` and `--include-sources` are omitted, default is `sources`.
- `--group-by component` requires `--component-map`.
- `--max-files` and `--max-bytes` are both allowed; the writer must satisfy both limits.
- If a single file exceeds `--max-bytes`, the run fails with a non-zero exit code.

## Exit codes
- `0`: Success.
- `1`: Invalid CLI usage or conflicting options.
- `2`: I/O failure (read/write).
- `3`: Traversal failure (permissions, symlink loops, etc.).
- `4`: Output constraints could not be satisfied.

## Examples
```
repaddu --input . --output ./out
repaddu -i /path/repo -o /tmp/out --group-by type --headers-first
repaddu -i . -o out --max-bytes 200000 --max-files 12
repaddu -i . -o out --extensions h,cpp --emit-cmake --emit-tree
```
