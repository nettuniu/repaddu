# format

Purpose:
- Output rendering/writing for markdown/jsonl/html and analysis reports.

Primary code:
- `include/repaddu/format_writer.h`, `src/format_writer.cpp`
- `src/format_writer_internal.h`, `src/format_writer_markdown.cpp`, `src/format_writer_plan.cpp`
- `src/format_writer_read.cpp`, `src/format_writer_alt_formats.cpp`
- `include/repaddu/format_tree.h`, `src/format_tree.cpp`
- `include/repaddu/format_language_report.h`, `src/format_language_report.cpp`
- `include/repaddu/format_analysis_report.h`, `src/format_analysis_report.cpp`
- `include/repaddu/format/analysis_tags_report.h`, `src/format/analysis_tags_report.cpp`
- `include/repaddu/format_analysis_json.h`, `src/format_analysis_json.cpp`

Dependencies:
- Internal: `repaddu_base`, `repaddu_analysis`.
- External: standard library only.

Invariants:
- Preserve output structure and ordering for existing modes.
- Must not depend on `cli` or entrypoint.
