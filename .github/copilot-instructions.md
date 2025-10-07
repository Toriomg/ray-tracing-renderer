# Copilot Instructions for render-2025-a-m81-10

## Project Overview
This is a C++ project for the Computer Architecture course at Universidad Carlos III de Madrid. The codebase is organized into several modules for different rendering architectures and utilities:
- `aos/`, `soa/`: Main implementations for Array of Structures and Structure of Arrays approaches. Each has its own `main.cpp`.
- `common/`: Shared code, including math, scene parsing, and data structures (see `include/` and `src/`).
- `ut*` folders: Unit test projects for each main module (e.g., `utsoa/`, `utcommon/`).
- `docs/`: Design documents and diagrams.

## Build & Test Workflow
- **CMake** is used for builds. The root `CMakeLists.txt` configures all subprojects.
- To build all targets:
  ```powershell
  cmake -S . -B build
  cmake --build build
  ```
- To run tests (example for SOA):
  ```powershell
  build\utsoa\utsoa.exe
  ```
- Each module and test has its own CMakeLists.txt. Update these when adding new files.

## Key Conventions & Patterns
- **Data Structures:**
  - Core types (e.g., `vec3`, `aabb`, `material`, `object`) are in `common/include/` and used across modules.
  - Scene parsing logic is in `common/src/scene_parser.cpp`.
- **Math Functions:**
  - Shared math utilities are in `common/src/mathFunctions.cpp`.
- **Testing:**
  - Unit tests are standalone executables in `ut*` folders. No external test framework is used; tests are typically run manually.
- **Documentation:**
  - Design and function documentation is in `docs/` (see `ECSdesign.md`, `WritePixelColor.md`).

## Integration Points
- No external dependencies beyond standard C++ and CMake.
- All cross-module communication uses shared headers in `common/include/`.

## Example: Adding a New Data Structure
1. Add header to `common/include/dataStructs/`.
2. Implement logic in `common/src/`.
3. Update relevant `CMakeLists.txt` files.
4. Add/modify tests in the appropriate `ut*` folder.

## Tips for AI Agents
- Always update CMakeLists.txt when adding/removing source files.
- Prefer using shared types from `common/include/` for interoperability.
- Reference design docs in `docs/` for architectural decisions.
- Build and test frequently to catch integration issues early.

---
_Last updated: 2025-10-06_
