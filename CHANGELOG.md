# Changelog

Main message: repaddu evolved from initial CLI export tooling into a modular, tested analysis-capable system with safety, configuration, multi-format output, and CI hardening.

Detailed message: sections below provide a per-day main message followed by the full detailed commit list.

## 2026-02-01
Main message: Initial foundation: core CLI, grouping/writer pipeline, early docs, scripts, CI bootstrap, and language/build profiling.
Detailed changes:
- `104626a` **daniele**: adding agent files
- `b8f4f05` **daniele**: addig docs
- `f835fe6` **vm_agent**: Implement repaddu CLI, grouping, writer, and tests
- `2f83a5c` **vm_agent**: Add CI workflow
- `4fdbcd9` **vm_agent**: Add build and run helper scripts
- `39a5fb2` **vm_agent**: Add language scan report option
- `c08fb05` **vm_agent**: Add language/build profiles and build-file aggregation

## 2026-02-07
Main message: Major feature wave: analysis/UI/config/HTML capabilities delivered as a large integrated phase.
Detailed changes:
- `71ceaa6` **vm_agent**: feat: Implement Phases 2-4 (Analysis, UI, Config, HTML)

## 2026-02-08
Main message: Advanced analysis expansion: graph/views plus LSP/C++ analysis support landed.
Detailed changes:
- `3d0a56b` **daniele**: Add analysis graph, views, and LSP/C++ analysis support

## 2026-02-12
Main message: Capability broadening: analysis quality improvements, fixtures/tests, docs, detection, and evaluation assets.
Detailed changes:
- `90c775d` **vm_agent**: Improve analysis CLI/config handling and aggregation output
- `14cecde` **vm_agent**: Add fixture-based analysis integration tests
- `c843733` **vm_agent**: Document pinned picobench submodule fixture
- `d35f08b` **vm_agent**: Add comprehensive docs guide
- `67e6738` **vm_agent**: Gate LSP deep relationship edges by options
- `808bb3b` **vm_agent**: Add LSP client initialize/documentSymbol helpers
- `7c108be` **vm_agent**: Add analysis views to JSON analysis output
- `aa04289` **vm_agent**: Add auto-detection for language and build system
- `39948a3` **vm_agent**: Add multi-language fixtures and integration tests
- `479a0d8` **vm_agent**: Make Clang analyzer option fail-soft and expand deep-edge test
- `006a544` **vm_agent**: Add traversal concurrency determinism test
- `7d07168` **vm_agent**: Add agent-evaluation report template and presets
- `4993b04` **vm_agent**: Add configurable config path for load and init
- `5724b68` **vm_agent**: Document configurable config path support
- `5680efc` **vm_agent**: Add file-based custom tag pattern loading

## 2026-02-13
Main message: Hardening sprint: config/YAML robustness, CLI/help clarity, CI portability, Windows fixes, and extensive test coverage.
Detailed changes:
- `af42614` **vm_agent**: Ensure isolate-docs applies to size grouping
- `467ea46` **vm_agent**: Add optional tag summary in analyze-only reports
- `3878edc` **vm_agent**: Add optional YAML frontmatter emission for output blocks
- `e4b2aad` **vm_agent**: Add no-links option for overview table of contents
- `8d14c97` **vm_agent**: Add analyze-only integration test coverage
- `6b3e94a` **vm_agent**: Add YAML config loading support
- `15184c5` **vm_agent**: Add dry-run integration test
- `6fb8ed1` **vm_agent**: Make logging and PII test commands explicit
- `db4257c` **vm_agent**: Add npm build-system detection and aggregation
- `31e5f5e` **vm_agent**: Emit YAML config when --init path uses yaml extension
- `83a0a06` **vm_agent**: Strengthen config path tests with content checks
- `2422d46` **vm_agent**: Document config format selection for --init
- `57716b7` **vm_agent**: Cover yml and uppercase YAML config generation
- `7a4fda7` **vm_agent**: Ignore CTest Testing output directory
- `98ffd9b` **vm_agent**: Test generated JSON and YAML config default equivalence
- `7bef4ab` **vm_agent**: Add help text regression for config generation formats
- `bff045e` **vm_agent**: Support inline YAML comments in config parser
- `85f5d49` **vm_agent**: Preserve quoted hash values in YAML config parsing
- `f0efc54` **vm_agent**: Document expanded config test coverage in TEST_INFO
- `b02c432` **vm_agent**: Expand language profile tests for build file resolution
- `0429dd1` **vm_agent**: Add language profile edge-case coverage
- `da2f27a` **vm_agent**: Refactor and test config auto-load path precedence
- `633589d` **vm_agent**: Test config path fallback when --config value is missing
- `01c313f` **vm_agent**: Add case-insensitive profile lookup tests
- `c58fad6` **vm_agent**: Add YAML escaped-quote inline comment regression test
- `2c7195f` **vm_agent**: Clarify auto-load config precedence in CLI help
- `e8b5118` **vm_agent**: Lock first-match behavior for repeated --config
- `2ab71fc` **vm_agent**: CI: checkout submodules for picobench fixture test
- `efa4a9b` **vm_agent**: CI: run build and tests on Linux, macOS, and Windows
- `450c53b` **vm_agent**: Fix Windows std::numeric_limits max macro conflict
- `ba26561` **vm_agent**: Harden Windows build by defining NOMINMAX
- `1ccd797` **vm_agent**: Expand JSONL writer coverage in output tests
- `c28517e` **vm_agent**: Add CLI/config format parsing coverage
- `adcf4a5` **vm_agent**: Test format precedence between config and CLI

## 2026-02-14
Main message: Architecture cleanup: component splits/refactors, test target deduplication, and documentation alignment.
Detailed changes:
- `e7a0e83` **vm_agent**: refactor(cli): split cli_parse into parse/config/help units
- `7bfe9d1` **vm_agent**: build(cmake): add test helper and deduplicate test targets
- `8dcdd4e` **vm_agent**: refactor(app): extract run orchestration and stream tag scanning
- `a8e4d2f` **vm_agent**: docs(cli): set cli_spec as canonical option reference
- `8e5c054` **vm_agent**: docs(test): align TEST_INFO with active test matrix
- `ceedf78` **vm_agent**: refactor(format): split jsonl/html writers from format_writer
- `eb063f1` **vm_agent**: test(app): add app_run orchestration lifecycle coverage
- `e575a7c` **vm_agent**: docs(perf): add repeatable baseline script and update eval report

## 2026-02-16
Main message: Stabilization: docs/spec sync, evaluation smoke tooling, and JSONL newline correctness fix.
Detailed changes:
- `f95f2b4` **vm_agent**: docs(cli): align spec with implemented flags and add perf guard
- `138618a` **vm_agent**: docs(eval): add smoke runner and update scenario execution status
- `dbbc2ee` **vm_agent**: fix(format): emit real newline delimiters in jsonl output

## 2026-02-19
Main message: Large refactor execution day: changelog formalization plus AGENTS-aligned architecture refactors (tasks 1-10 in progress/complete).
Detailed changes:
- `0ddc6d5` **vm_agent**: analysis: support @todo-style tags with safe matching
- `ed7f94b` **vm_agent**: docs: add full historical changelog from repository history
- `2b8a1e7` **vm_agent**: cli: accept --token-count as compatibility flag
- `ab68623` **vm_agent**: docs(changelog): add main and detailed messages per date
- `d670070` **vm_agent**: refactor: start AGENTS alignment tasks 1-4
- `4b745d0` **vm_agent**: refactor(core): split base and analysis libraries
- `42d52e4` **vm_agent**: refactor: continue tasks 71-75 with safe structural splits
- `946c97e` **vm_agent**: refactor: advance tasks 76-80 with app boundaries and policy test
