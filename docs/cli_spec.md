# CLI Specification

This is the canonical source for CLI flags, defaults, and constraints.

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
  - Required unless using `--init` / `--generate-config`.
- `-o, --output <path>`
  - Output directory where generated files are written.
  - Optional for `--scan-languages`, `--analyze-only`, and `--init` / `--generate-config`.

### Output sizing and numbering
- `--max-files <count>`
  - Maximum number of output files to generate.
  - Default: `0` (no limit).
- `--max-bytes <bytes>`
  - Maximum bytes per output file (content bytes, not filesystem size).
  - Default: `0` (no limit).
- `--number-width <n>`
  - Width of numeric prefix (e.g., `3` -> `000`).
  - Default: `3`.

### Filtering and traversal
- `--include-headers`
  - Include header files only (`.h`, `.hpp`, `.hh`, `.hxx`) unless `--include-sources` is also provided.
  - Default: `false`.
- `--include-sources`
  - Include source files (`.c`, `.cc`, `.cpp`, `.cxx`).
  - Default: `true` when neither include flag is provided.
- `--extensions <csv>`
  - Comma-separated list of extensions to include (overrides include-headers/include-sources).
  - Default: empty.
- `--exclude-extensions <csv>`
  - Comma-separated list of extensions to exclude (applied after includes).
  - Default: empty.
- `--include-hidden`
  - Include hidden files and directories.
  - Default: `false`.
- `--follow-symlinks`
  - Follow directory symlinks.
  - Default: `false`.
- `--single-thread`
  - Force single-threaded traversal.
  - Default: `false` (equivalent to setting `parallel_traversal=false`).
- `--parallel-traversal`
  - Enable parallel traversal.
  - Default: `true`.

### Safety and size guards
- `--include-binaries`
  - Include binary files (normally excluded).
  - Default: `false`.
- `--max-file-size <bytes>`
  - Skip files larger than this threshold.
  - Default: `1048576` (1MB).
- `--force-large`
  - Include files above `--max-file-size`.
  - Default: `false`.
- `--redact-pii`
  - Redact emails, IPs, and common secret-like patterns in emitted content.
  - Default: `false`.

### Analysis and run modes
- `--scan-languages`
  - Scan repository and report language percentages only (no output files).
  - Default: `false`.
- `--analyze-only`
  - Run analysis/statistics without generating output files.
  - Default: `false`.
- `--analysis`
  - Enable symbol/dependency analysis (AST/LSP-backed when available).
  - Default: `false`.
- `--analysis-views <csv>`
  - Comma-separated analysis views to emit.
  - Default: empty.
- `--analysis-deep`
  - Enable deeper relationship analysis.
  - Default: `false`.
- `--analysis-collapse <mode>`
  - Collapse level for analysis output.
  - Allowed values: `none`, `folder`, `target`.
  - Default: `none`.
- `--extract-tags`
  - Include TODO/FIXME-like tag summary in analyze output.
  - Default: `false`.
- `--tag-patterns <path>`
  - Load additional tag patterns from file (one pattern per line, `#` comments allowed).
  - Default: empty.
- `--isolate-docs`
  - Group documentation files (`*.md`, `*.txt`) into a separate chunk.
  - Default: `false`.
- `--dry-run`
  - Simulate execution without writing output files.
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
  - Emit a recursive tree listing section.
  - Default: `true`.
- `--emit-cmake`
  - Aggregate all `CMakeLists.txt` into a dedicated output file.
  - Default: `true`.
- `--emit-build-files`
  - Aggregate build-system files (based on selected language/build-system profiles).
  - Default: `false`.
- `--no-links`
  - Disable markdown links in overview table of contents.
  - Default: links enabled.
- `--markers <mode>`
  - Boundary marker style for file content blocks.
  - Allowed values: `fenced`, `sentinel`.
  - Default: `fenced`.
- `--frontmatter`
  - Add YAML frontmatter metadata before each emitted file content block.
  - Default: `false`.
- `--format <fmt>`
  - Output format.
  - Allowed values: `markdown`, `jsonl`, `html`.
  - Default: `markdown`.

### Language and build-system profiles
- `--language <id>`
  - Explicit language profile.
  - Allowed values: `auto`, `c`, `cpp`, `rust`, `python`.
  - Default: `auto`.
- `--build-system <id>`
  - Explicit build-system profile.
  - Allowed values: `auto`, `cmake`, `make`, `meson`, `bazel`, `cargo`, `npm`, `python`.
  - Default: `auto`.

### Config and misc
- `--config <path>`
  - Config file path to load and/or generate (`.json`, `.yaml`, `.yml`).
  - Default target for generation: `.repaddu.json`.
- `--init`, `--generate-config`
  - Generate a default config at `--config` path.
  - Output format follows extension (`.json`, `.yaml`, `.yml`), default `.repaddu.json`.
- `-h`, `--help`
  - Show help and exit with code 0.
- `--version`
  - Show version and exit with code 0.

## Precedence and constraints
- Config precedence: defaults < config file (`--config` path or auto-loaded `.repaddu.json`/`.repaddu.yaml`/`.repaddu.yml`) < CLI flags.
- `--extensions` overrides `--include-headers` and `--include-sources`.
- If both `--include-headers` and `--include-sources` are omitted, include-sources defaults to enabled.
- `--group-by component` requires `--component-map`.
- `--analysis-collapse` must be `none`, `folder`, or `target`.
- `--markers` must be `fenced` or `sentinel`.
- `--format` must be `markdown`, `jsonl`, or `html`.
- `--language` must be `auto` or a registered language profile.
- `--build-system` must be `auto` or a registered build-system profile.
- `--max-files` must be a non-negative integer.
- `--max-bytes` and `--max-file-size` must be non-negative integers.
- `--number-width` and `--group-depth` must be positive integers.

## Exit codes
- `0`: Success.
- `1`: Invalid CLI usage or conflicting options.
- `2`: I/O failure (read/write).
- `3`: Traversal failure (permissions, symlink loops, etc.).
- `4`: Output constraints could not be satisfied.

## Examples
```bash
repaddu --input . --output ./out
repaddu -i . --scan-languages
repaddu -i . --analyze-only --extract-tags
repaddu -i . -o out --group-by component --component-map components.json
repaddu -i . -o out --redact-pii --max-file-size 524288
repaddu -i . -o out --format jsonl --analysis --analysis-views symbols,dependencies
```
