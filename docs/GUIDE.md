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

Canonical source for flags, defaults, constraints, and examples:

- `docs/cli_spec.md`

Quick syntax reminder:

```bash
repaddu [options] --input <path> --output <path>
```

Operational notes:

- `--output` is optional for `--scan-languages`, `--analyze-only`, and `--init`.
- Auto-load order for config in the current directory is `.repaddu.json`, `.repaddu.yaml`, then `.repaddu.yml`.
- CLI flags override config values.

## Configuration Reference

Config files: `.repaddu.json`, `.repaddu.yaml`, or `.repaddu.yml`.
Auto-load precedence in current working directory: `.repaddu.json` then `.repaddu.yaml` then `.repaddu.yml`.
Use `--config <path>` to override the default config location.

### Precedence

- Defaults (built-in) < auto-loaded config (`.repaddu.json`, `.repaddu.yaml`, `.repaddu.yml`) < CLI flags

### Generate starter config

```bash
repaddu --init
```

Generate at a custom path:

```bash
repaddu --config ./configs/project.repaddu.json --init
```

Generate YAML config:

```bash
repaddu --config ./configs/project.repaddu.yaml --init
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
- `emit_links` (bool)
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

### Refactor migration map (for contributors)

The repository now follows a layered target model. Use this map when deciding
where new code belongs or where to move legacy logic:

- `repaddu_base`: shared domain types/utilities with no app knowledge.
- `repaddu_analysis`: analysis graph/view/LSP logic.
- `repaddu_io`: filesystem traversal and binary detection.
- `repaddu_grouping`: grouping/filtering strategy and component map logic.
- `repaddu_format`: markdown/jsonl/html and analysis report renderers.
- `repaddu_ui`: UI abstraction and console adapter.
- `repaddu_cli`: CLI parse/config/bootstrap and run orchestration.
- `repaddu` executable: thin entrypoint only (`src/main.cpp`).

Migration compatibility rules currently in effect:

- Keep compatibility headers in `include/repaddu/*.h` working while migrating
  internal implementation files.
- Keep behavior identical during refactors (output bytes, ordering, errors).
- Preserve dependency direction from `docs/architecture_layers.md`; do not
  introduce reverse edges or cycles.

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
