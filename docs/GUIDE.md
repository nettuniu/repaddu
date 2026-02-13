# The Huge Guide

This guide is the full user and contributor manual for `repaddu`.

## Installation

### Prerequisites

- CMake 3.20+
- A C++20 compiler (`g++`/`clang++`)
- `git`

Optional for C++ AST analysis tests/backend:

- LLVM/Clang development packages (for `REPADDU_ENABLE_CLANG=ON`)

### Clone

```bash
git clone git@github.com:nettuniu/repaddu.git
cd repaddu
```

If you want fixture tests that use the external project submodule:

```bash
git submodule update --init --recursive fixtures/picobench
```

### Build

```bash
cmake -S . -B build
cmake --build build -j
```

Binary path:

- `build/repaddu`

Run quick smoke test:

```bash
./build/repaddu --help
```

### Optional: Enable Clang-based C++ analyzer

```bash
cmake -S . -B build-clang -DREPADDU_ENABLE_CLANG=ON
cmake --build build-clang -j
```

If configuration fails, verify local Clang CMake packages and static libraries are complete.

## CLI Reference

### Basic syntax

```bash
repaddu [options] --input <path> --output <path>
```

Notes:

- `--output` is optional for `--scan-languages`, `--analyze-only`, and `--init`.
- By default `.repaddu.json` in the current directory is loaded first.
- Use `--config <path>` to load a different config file and to choose the target path for `--init`.
- CLI args override config values.

### Core options

- `-i, --input <path>`: Input folder/repository.
- `-o, --output <path>`: Output directory for generated files.
- `--max-files <count>`: Max output file count (`0` = unlimited).
- `--max-bytes <bytes>`: Max bytes per output file (`0` = unlimited).
- `--number-width <n>`: Output numeric prefix width.

### Filtering and traversal

- `--include-headers`: Include header files.
- `--include-sources`: Include source files.
- `--extensions <csv>`: Explicit include extension list.
- `--exclude-extensions <csv>`: Exclude extensions after include filtering.
- `--include-hidden`: Include dotfiles/directories.
- `--follow-symlinks`: Follow directory symlinks.
- `--single-thread`: Disable parallel traversal.
- `--parallel-traversal`: Enable parallel traversal.
- `--include-binaries`: Include binary files.
- `--max-file-size <bytes>`: Skip files above this size (default `1048576`).
- `--force-large`: Do not skip large files.

### Grouping and output layout

- `--group-by <mode>`: `directory|component|type|size`.
- `--group-depth <n>`: Directory depth for `directory` grouping.
- `--component-map <path>`: Required for `component` grouping.
- `--headers-first`: Place headers before sources when ordering applies.
- `--emit-tree`: Include recursive tree listing.
- `--emit-cmake`: Include aggregated `CMakeLists.txt` output.
- `--emit-build-files`: Include aggregated build-system file output.
- `--markers <mode>`: `fenced|sentinel` block boundary style.
- `--frontmatter`: Add YAML frontmatter metadata before each file content block.
- `--format <fmt>`: `markdown|jsonl|html`.

### Analysis options

- `--scan-languages`: Print language percentage report to stdout.
- `--analyze-only`: Print analysis report only; no output files.
- `--analysis`: Enable analysis mode.
- `--analysis-views <csv>`: Select analysis views.
- `--analysis-deep`: Enable deeper relationship extraction.
- `--analysis-collapse <mode>`: `none|folder|target`.
- `--extract-tags`: Include TODO/FIXME-like tag summary in analyze output.
- `--tag-patterns <path>`: Load additional tag patterns from file (one per line).
- `--isolate-docs`: Put docs (`.md`, `.txt`) in a separate chunk.
- `--redact-pii`: Redact common PII patterns.
- `--dry-run`: Simulate run without writing files.

### Config/bootstrap and misc

- `--init` or `--generate-config`: Create default `.repaddu.json` in current folder.
- `--config <path>`: Config path to load and/or generate (default `.repaddu.json`).
- `--language <id>`: `auto|c|cpp|rust|python`.
- `--build-system <id>`: `auto|cmake|make|meson|bazel|cargo|python`.
- `-h, --help`: Print help.
- `--version`: Print version string.

### Exit codes

- `0`: success
- `1`: invalid usage
- `2`: I/O failure
- `3`: traversal failure
- `4`: output constraints failure

## Configuration Reference

Config file: `.repaddu.json` (loaded automatically from current working directory if present).
Use `--config <path>` to override the default config location.

### Precedence

- Defaults (built-in) < `.repaddu.json` < CLI flags

### Generate starter config

```bash
repaddu --init
```

Generate at a custom path:

```bash
repaddu --config ./configs/project.repaddu.json --init
```

### Supported keys

- `input` (string path)
- `output` (string path)
- `max_files` (integer)
- `max_bytes` (integer)
- `number_width` (integer)
- `include_headers` (bool)
- `include_sources` (bool)
- `include_hidden` (bool)
- `include_binaries` (bool)
- `follow_symlinks` (bool)
- `headers_first` (bool)
- `emit_tree` (bool)
- `emit_cmake` (bool)
- `emit_build_files` (bool)
- `frontmatter` (bool)
- `max_file_size` (integer)
- `force_large` (bool)
- `redact_pii` (bool)
- `analyze_only` (bool)
- `analysis_enabled` (bool)
- `analysis_views` (array of strings)
- `analysis_deep` (bool)
- `analysis_collapse` (`none|folder|target`)
- `extract_tags` (bool)
- `tag_patterns` (string path)
- `isolate_docs` (bool)
- `dry_run` (bool)
- `parallel_traversal` (bool)
- `format` (`markdown|jsonl|html`)
- `group_by` (`directory|component|type|size`)
- `markers` (`fenced|sentinel`)
- `extensions` (array of strings)
- `exclude_extensions` (array of strings)

### Example config

```json
{
  "input": ".",
  "output": "repaddu_out",
  "group_by": "directory",
  "number_width": 3,
  "emit_tree": true,
  "emit_cmake": true,
  "analysis_enabled": false,
  "analysis_views": [],
  "extensions": [],
  "exclude_extensions": []
}
```

## Recipes (Different LLMs)

### ChatGPT / Codex context pack (general code review)

Goal: produce concise, component-oriented markdown chunks.

```bash
repaddu \
  --input . \
  --output out_chatgpt \
  --group-by component \
  --component-map components.json \
  --emit-tree \
  --emit-cmake \
  --headers-first
```

### Claude long-context map-first pass

Goal: strong repository topology before deep file reading.

```bash
repaddu \
  --input . \
  --output out_claude \
  --group-by directory \
  --group-depth 2 \
  --emit-tree \
  --max-bytes 220000
```

### Gemini / broad summarization pass

Goal: language/build metadata plus grouped source chunks.

```bash
repaddu --input . --scan-languages

repaddu \
  --input . \
  --output out_gemini \
  --group-by type \
  --emit-build-files \
  --language cpp \
  --build-system cmake
```

### Analysis-only report for agent planning

Goal: quickly summarize structure without writing chunk files.

```bash
repaddu \
  --input . \
  --analyze-only \
  --format jsonl \
  --analysis \
  --analysis-collapse folder
```

### Safety-first export for shared prompts

Goal: avoid accidental exposure of sensitive strings and huge files.

```bash
repaddu \
  --input . \
  --output out_safe \
  --redact-pii \
  --max-file-size 524288 \
  --exclude-extensions lock,key,p12
```

## Contributor Guide

### Project layout

- `include/repaddu/`: public headers
- `src/`: implementation
- `tests/`: test executables
- `fixtures/`: test fixtures and submodules
- `docs/`: documentation

### Build and test workflow

1. Configure and build:

```bash
cmake -S . -B build
cmake --build build -j
```

2. Run full tests:

```bash
ctest --test-dir build --output-on-failure
```

3. Or run area tests from `TEST_INFO.txt`:

```bash
ctest --test-dir build -R repaddu_test_filtering
ctest --test-dir build -R repaddu_test_cli_parse_analysis
```

### Testing policy

- `TEST_INFO.txt` is the mapping source of truth for code area -> tests.
- If you add/modify tests, update `TEST_INFO.txt` in the same change.
- When changing code, run the mapped tests for that area.

### Style and architecture expectations

- Keep behavior unchanged for refactors unless explicitly requested.
- Prefer small, testable units and clear interfaces.
- Keep GUI/UI surface minimal; place logic in non-UI libraries.
- Preserve layering and avoid dependency cycles.
- Keep edits scoped; avoid broad reformatting.

### Submodules in tests

For external fixture repositories (for example `fixtures/picobench`), always initialize submodules before running related tests:

```bash
git submodule update --init --recursive
```
