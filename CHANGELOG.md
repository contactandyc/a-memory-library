## 08/22/2025

**Modernize A Memory Library: build system, docs, SPDX headers, tests**

---

## Summary

This PR modernizes **A Memory Library (AML)** with a new CMake system, simplified Docker setup, full per-component documentation, improved allocator semantics, and a comprehensive test suite with coverage support.

---

## Key Changes

### 🔧 Build & Tooling

* **Removed** legacy files:

    * `CHANGELOG.md`, `NEWS.md`, `build_install.sh`, `.changie.yaml`, `.changes/`.
* **Added** `build.sh`:

    * Commands: `build`, `install`, `coverage`, `clean`.
    * Coverage integration with `llvm-cov`.
* `.gitignore`: added `build-unix_makefiles`, `build-cov`, `build-coverage`.
* **CMake**:

    * Minimum version raised to **3.20**.
    * Project renamed to `a_memory_library` (underscore convention).
    * **Multi-variant builds**: `debug`, `memory`, `static`, `shared`.
    * Umbrella alias: `a_memory_library::a_memory_library`.
    * Coverage toggle (`A_ENABLE_COVERAGE`) and memory profiling define (`_AML_DEBUG_`).
    * Proper **install/export** with generated config + version files.
* **Dockerfile**:

    * From `ubuntu:22.04` with configurable CMake version.
    * Installs system dependencies + optional dev tools.
    * Non-root `dev` user with passwordless sudo.
    * Builds/install only AML (no external repos).

### 📖 Documentation

* **AUTHORS**:

    * Updated format, expanded Andy Curtis entry with GitHub profile.
* **NOTICE**:

    * Simplified attribution:

        * Andy Curtis (2019–2025), Knode.ai (2024–2025).
    * Removed inline “technical contact” note.
* **BUILDING.md**:

    * Modernized for v0.0.1.
    * Clear local build and install instructions.
    * Simplified Docker instructions.
* **README.md**:

    * Rewritten to highlight AML’s three core components:

        * `aml_alloc`, `aml_pool`, `aml_buffer`.
    * “Hello AML” example provided.
    * Links to new per-component READMEs.
* **New detailed docs**:

    * `README.aml_alloc.md` — safe allocation wrappers.
    * `README.aml_buffer.md` — auto-growing NUL-terminated buffer.
    * `README.aml_pool.md` — arena allocator, helpers, Base64, splits.

### 📝 Source & Headers

* SPDX headers updated:

    * Now use en-dash (`2019–2025`).
    * Andy Curtis explicitly credited.
    * Knode.ai tagged with “technical questions” note.
* **`aml_alloc`**:

    * Added safe `_aml_free` inline wrapper.
    * `strdupan` updated: now **preserves NULL entries** and truncates safely.
    * Debug logging: re-invoking `_aml_alloc_log` replaces filename cleanly.
* **`aml_buffer`**:

    * Stronger `detach` semantics:

        * Heap-backed: returns owned buffer or creates minimal safe buffer.
        * Pool-backed: returns pool memory (not freed).
    * `destroy` checks sentinel vs. heap.
* **`aml_pool`**:

    * `strdupan` improved: preserves NULL entries.
    * `count_bytes_in_array` correctly updates `*n`.
* Minor API clarifications in inline headers.

### ✅ Tests

* **`tests/CMakeLists.txt`**:

    * Builds test executables for alloc, buffer, pool.
    * Variant-aware linking with coverage aggregation.
* **`tests/build.sh`**:

    * Supports variants (`debug|memory|static|shared|coverage`).
    * Auto job detection.
* **New tests**:

    * `test_aml_alloc.c`: malloc/calloc/zalloc/realloc, strdup family, strdupa variants (including NULL entries), zero-length allocs, realloc ping-pong, large allocs, edge cases.
    * `test_aml_buffer.c`: sets/appends/resizes, detach semantics (heap vs pool), alignment guarantees, reset, growth cycles, binary data with NULLs.
    * `test_aml_pool.c`: alloc/calloc/ualloc/aalloc, markers, clear, subpools, split (with/without escape), Base64 round-trip, strdupa(n/2), lifecycle behaviors.

---

## Impact

* 🚀 Unified modern build (CMake 3.20+, multi-variant).
* 📦 Simplified Docker and local dev workflow.
* 🛡️ Safer allocators (`aml_alloc`, `aml_buffer`, `aml_pool`) with explicit NULL-preservation and detach semantics.
* 📖 Rich per-component docs for adoption.
* ✅ Full regression test coverage with debug/release semantics.
