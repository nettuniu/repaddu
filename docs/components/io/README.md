# io

Purpose:
- Filesystem traversal and binary detection adapters.

Primary code:
- `include/repaddu/io_traversal.h`, `src/io_traversal.cpp`
- `include/repaddu/io_binary.h`, `src/io_binary.cpp`

Dependencies:
- Internal: `repaddu_base`.
- External: standard library filesystem.

Invariants:
- Keep traversal semantics deterministic for equivalent inputs.
- Avoid dependencies on `grouping`, `format`, `ui`, or `cli`.
