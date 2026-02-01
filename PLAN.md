# GOAL
Build a C++ CLI application that converts a repository/folder into a sequence of Markdown files suitable for LLM/RAG ingestion (e.g., NotebookLM).

# OUTPUT FORMAT
- Output files are numbered and named: `000_category_filespecific.md`, `001_...`, etc.
- File `000_*.md` explains how to interpret all other files (markers, boundaries, conventions).
- Every other output file starts with a short note: “Read `000_*.md` first.”

# CORE REQUIREMENTS
- Ignore `.git` files and folders.
- Exclude binaries by default (must be enforced).
- Provide a recursive file listing (ls-like tree) in a dedicated output section.
- Collect all `CMakeLists.txt` files into a single output file, each preceded by its path.

# CLI REQUIREMENTS
- Input path and output path options.
- Output chunking options (e.g., number of files, max size per file).
- Filtering options:
  - Include only headers (`.h/.hpp`) or include source files (`.cpp/.cc/.cxx`) too.
  - Option to group all headers first and all sources after.
- Options that control how libraries are grouped (see next section).

# LIBRARY GROUPING OPTIONS (NEED MULTIPLE STRATEGIES)
Provide switches for alternative grouping strategies, e.g.:
- By directory (top-level folders).
- By library/component (configurable mappings).
- By file type (headers vs sources).
- By size (balance to keep files similar length).

# ARCHITECTURE REQUIREMENTS
- Modular design with libraries (no monolithic main).
- Minimal code in `main` (just CLI parsing + orchestration).
- Clear separation of parsing, filtering, grouping, and output formatting.

# GITHUB TASKS (DETAILED)
1) Define CLI spec and defaults
   - Enumerate flags, defaults, and examples.
   - Decide short/long option names and help text.
   - Define how conflicting flags are resolved.
2) Define file type policy and binary exclusion
   - List default include extensions.
   - Specify a binary detection heuristic (size, NUL bytes).
   - Add explicit allow/deny lists via CLI.
3) Define output naming and numbering rules
   - Specify numbering width and padding behavior.
   - Define category naming rules and sanitization.
   - Define stable ordering guarantees.
4) Write `000_*.md` specification template
   - Define markers that delimit files/sections.
   - Describe metadata headers for each entry.
   - Include example of one encoded file.
5) Add “read 000 first” boilerplate for all other outputs
   - Define the exact sentence and placement.
   - Ensure it is added by the writer layer.
6) Implement repository traversal and `.git` exclusion
   - Use filesystem traversal with `.git` skipping.
   - Ensure symlink handling is explicit.
7) Implement binary file exclusion
   - Apply binary heuristic before reading.
   - Allow override via CLI (include-binary=false by default).
8) Implement file filtering by extension
   - Support headers-only, sources-only, or both.
   - Allow custom extension list via CLI.
9) Implement “headers first, then sources” grouping
   - Ensure stable ordering within each group.
   - Keep other grouping strategies compatible.
10) Implement recursive “ls-like” listing output
   - Generate a deterministic tree listing.
   - Provide option to include/exclude hidden files.
11) Aggregate all `CMakeLists.txt` files
   - Find all matches, store sorted by path.
   - Prefix each with its path in output.
12) Grouping strategy: by directory
   - Group by top-level folder (configurable depth).
   - Ensure empty groups are not emitted.
13) Grouping strategy: by library/component map
   - Define a mapping file format (YAML/JSON/TOML).
   - Resolve file → component mapping rules.
14) Grouping strategy: by file type
   - Group by extension class (headers/sources/other).
   - Provide explicit ordering of classes.
15) Grouping strategy: size-balanced chunks
   - Implement bin-packing heuristic.
   - Ensure deterministic output for same input.
16) Chunk sizing options
   - Support max files per output and/or max bytes.
   - Handle overflow cases with clear error messages.
17) Output writer abstraction
   - Define interfaces for writer, formatter, and serializer.
   - Ensure new formats can be added without touching core.
18) Define content boundary markers
   - Choose markers that are LLM-friendly and unambiguous.
   - Ensure markers are documented in `000_*.md`.
19) Enforce architectural boundaries and layering
   - Define libs: core (model), io, grouping, formatting, cli.
   - Ensure dependencies flow one-way and are acyclic.
20) Unit tests: filtering rules
   - Create fixture repo with mixed file types.
   - Verify `.git` exclusion and extension filtering.
21) Unit tests: chunking and grouping
   - Test multiple grouping strategies.
   - Verify determinism across runs.
22) Unit tests: CMakeLists aggregation
   - Ensure all `CMakeLists.txt` are found and ordered.
23) Test fixtures
   - Add minimal sample repositories.
   - Include binary sample file for exclusion testing.
24) README usage examples
   - Add CLI examples for common scenarios.
   - Document grouping strategies and tradeoffs.
25) Update `TEST_INFO.txt`
   - Add new code areas and required tests.
   - Keep commands consistent with repository standards.
26) Build integration (CMake)
   - Add libraries and app target.
   - Keep main small; link libs properly.
27) Optional: CI script
   - Build and run unit tests.
   - Report artifacts or coverage if required.
