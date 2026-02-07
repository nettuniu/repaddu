# REPADDU PROJECT PLAN

# GOAL
Build a robust, cross-platform C++ CLI application that converts a repository/folder into a sequence of Markdown files optimized for LLM/RAG ingestion. The tool must be modular, highly configurable, and safe.

# OUTPUT FORMAT
- **Numbered Files:** `000_category_filespecific.md`, `001_...`
- **Summary File:** `000_*.md` contains instructions, metadata, and analysis.
- **Content:** Subsequent files contain code/text with "Read 000 first" headers.

# ARCHITECTURE & COMPLIANCE
- **Modularity:** Core logic separate from UI. TUI/Progress bars in a dedicated optional library (`repaddu_ui`).
- **Cross-Platform:** Code must be compatible with Linux, Windows, and macOS.
- **Safety:** PII redaction and secret detection with warnings.
- **Performance:** Parallel traversal (optional), mmap support, zero-copy writing.

# TASKS

## PHASE 1: Core Foundation & Safety
1.  **Logging System**: Implement a robust logger (`repaddu::Logger`) with levels (INFO, WARN, DEBUG).
    - Log magic byte detections, PII warnings, and skipped files.
2.  **Binary & Large File Handling**:
    - **Magic Bytes**: Use magic byte detection (custom or libmagic) to identify binaries. Log findings.
    - **Size Guard**: Check file sizes. If > Threshold (e.g., 1MB), warn and exclude by default.
    - **Force Include**: Add CLI option to force inclusion of large/binary files.
3.  **PII & Secret Redaction**:
    - Scan for emails, IP addresses, and common API key patterns.
    - Replace with `<REDACTED>` and issue a **WARNING** to the user/log.
4.  **CLI Configuration**:
    - Support `.repaddu.yaml` or `.json` for complex configs.
    - Tool works without config by default.
    - Option to generate a default config file in a custom location.

## PHASE 2: Analysis & LLM Optimization
5.  **Token Counting**: Implement BPE/heuristic tokenizer to estimate and report token usage per file/chunk.
6.  **Context-Aware Chunking**:
    - Split large files at logical boundaries (functions/classes) rather than hard byte limits.
7.  **Analysis Mode**:
    - Flag `--analyze-only`: Perform full scan, statistics, and checks *without* generating output files.
    - **Language Stats**: Report % of C++, Python, etc.
    - **Complexity**: (Optional) Heatmap/score for file complexity.
    - **Dependency Map**: (Optional) Text-based graph of includes/imports.
8.  **Tag Extractor (TODOs)**:
    - Extract `@todo`, `@fixme`, etc.
    - **Configurable**: Allow loading custom tag patterns from a file.
9.  **Metadata Frontmatter**: (Optional, default OFF) Add YAML frontmatter to code blocks (author, date, size).
10. **Documentation Isolation**: Option to group all `.md`/`.txt` files into a separate "Context" chunk.

## PHASE 3: Modular UX & TUI
11. **Dry Run Mode**: Simulate execution, reporting file counts, groups, and errors without writing.
12. **Modular UI Library**: Create `repaddu_ui` for rich terminal features.
    - **Default**: Standard CLI (text output).
    - **Rich**: Progress bars, colored trees, icons (via `repaddu_ui` if linked/enabled).
13. **Link Generation**: (Optional) Generate relative links in Summary pointing to specific chunks.
14. **HTML Report**: (Optional) Generate `index.html` navigable tree/report.

## PHASE 4: Build System & formats
15. **Modular Build Aggregator**:
    - Abstract build system parsing.
    - Support CMake (default), plus optional modules for `package.json`, `Cargo.toml`, `requirements.txt`.
16. **Output Formats**:
    - Markdown (Default).
    - JSONL (for fine-tuning).
17. **Packaging & Distribution**:
    - **Linux**: `.rpm`, `.deb`, Static Binary (Alpine/Musl).
    - **Windows**: `.exe` (ensure path handling `\` vs `/`).
    - **MacOS**: Universal binary/Homebrew formula.
    - **CI/CD**: Github Actions for cross-platform builds.

## PHASE 5: Testing & Documentation
18. **Massive Testing Suite**:
    - Unit tests for all modules.
    - Fuzz testing for parser and file reader.
    - Integration tests with "Agent Validation" (see `AGENT_TESTING_PLAN.md`).
    - `TEST_INFO.txt` must be kept updated.
19. **The "Huge Guide"**:
    - Comprehensive documentation covering:
        - Installation (all platforms).
        - CLI Reference (every flag).
        - Configuration (schema, examples).
        - Recipes (Optimizing for Gemini, GPT-4, Local LLMs).
        - Architecture internals for contributors.

## PHASE 6: Performance (Optimization)
20. **Parallel Traversal**:
    - Multithreaded directory scanning.
    - **Disable Option**: Flag to force single-threaded mode (debugging/determinism).
21. **I/O Optimization**:
    - Memory-mapped files (`mmap`) for reading.
    - Zero-copy buffer management for writing.