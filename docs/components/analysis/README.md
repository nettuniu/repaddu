# analysis

Purpose:
- Analysis domain logic for graph/view/LSP relationships.

Primary code:
- `include/repaddu/analysis_graph.h`, `src/analysis_graph.cpp`
- `include/repaddu/analysis_view.h`, `src/analysis_view.cpp`
- `include/repaddu/analysis_lsp.h`, `src/analysis_lsp.cpp`

Dependencies:
- Internal: `repaddu_base`.
- External: standard library only.

Invariants:
- Depend only on `base` among internal targets.
- No CLI/UI/orchestration knowledge.
