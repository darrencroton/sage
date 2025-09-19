<!-- Purpose: Record critical technical decisions -->
<!-- Update Rules:
- Append new entries to the EOF (use `cat << EOF >> ...etc`)!
- Focus on KEY decisions that impact current and upcoming development
- Only include decisions that are NOT covered in architecture.md
- 100-word limit per entry! 
- Include:
  • Today's date and phase identifier
  • Rationale
  • Impact assessment
-->

# Critical Architectural Decisions

*Recent critical decisions - historical decisions archived in `archive/decisions-phase*.md`*

2025-09-19: [Phase 1.1] CMake Executable Placement Strategy
- **Decision**: Move compiled executable to source directory rather than implementing complex path resolution
- **Rationale**: SAGE parameter files contain relative paths expecting execution from source directory; moving executable maintains compatibility without code changes
- **Impact**: Preserves scientific accuracy, eliminates path configuration complexity, enables immediate user adoption of CMake build system
