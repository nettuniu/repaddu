# base

Purpose:
- Shared domain and utility layer with no app/orchestration knowledge.

Primary code:
- `include/repaddu/core_types.h`, `src/core_types.cpp`
- `include/repaddu/language_profiles.h`, `src/language_profiles.cpp`
- `include/repaddu/logger.h`, `src/logger.cpp`
- `include/repaddu/pii_redactor.h`, `src/pii_redactor.cpp`
- `include/repaddu/analysis_tokens.h`, `src/analysis_tokens.cpp`
- `include/repaddu/analysis_tags.h`, `src/analysis_tags.cpp`
- `include/repaddu/json_lite.h`, `src/json_lite.cpp`

Dependencies:
- Internal: none.
- External: standard library only.

Invariants:
- No dependency on `analysis`, `io`, `grouping`, `format`, `ui`, `cli`, or entrypoint.
- Keep types/utilities reusable and side-effect behavior stable.
