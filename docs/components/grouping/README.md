# grouping

Purpose:
- Filtering/grouping strategies and component map resolution.

Primary code:
- `include/repaddu/grouping_strategies.h`, `src/grouping_strategies.cpp`
- `include/repaddu/grouping_component_map.h`, `src/grouping_component_map.cpp`

Dependencies:
- Internal: `repaddu_base`.
- External: standard library only.

Invariants:
- Input-to-group mapping behavior must remain stable under refactors.
- No dependency on `io`, `format`, `ui`, or `cli`.
