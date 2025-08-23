# A Memory Library (AML)

**Fast, simple building blocks for memory‑heavy C code.**
AML gives you three focused pieces you can use together or alone:

* **`aml_alloc`** – tiny allocation wrappers with **debug‑time** tracking and leak/bad‑free detection.
  → See: [`README.aml_alloc.md`](README.aml_alloc.md)
* **`aml_pool`** – a small, fast **arena/region allocator** for “allocate a bunch → clear/destroy” workflows.
  → See: [`README.aml_pool.md`](README.aml_pool.md)
* **`aml_buffer`** – an auto‑growing, NUL‑terminated **byte/string buffer**, optionally backed by a pool.
  → See: [`README.aml_buffer.md`](README.aml_buffer.md)

Use them to cut fragmentation and syscalls, make lifetimes obvious, and turn on deep diagnostics when you’re chasing bugs.

---

## Why AML?

* **Predictable performance.** Pools turn many small allocations into pointer bumps. Buffers grow geometrically and keep data contiguous.
* **Clean lifetimes.** Instead of sprinkling `free` calls, clear or destroy a pool; reuse buffers without juggling capacity.
* **Better debugging (when you want it).** In `_AML_DEBUG_` builds, AML can track allocations with file/line tags, catch double frees, and dump live objects.

---

## What’s inside (at a glance)

| Component    | What it is                                   | Ideal for                                  |
| ------------ | -------------------------------------------- | ------------------------------------------ |
| `aml_alloc`  | libc‑compatible API + optional debug tracker | Drop‑in allocator with leak/bad‑free guard |
| `aml_pool`   | Arena/region allocator                       | Parsers, batch jobs, per‑request scratch   |
| `aml_buffer` | Auto‑growing contiguous buffer (text/binary) | Builders/formatters, serialization         |

> Threading: pools and buffers are not thread‑safe; use one per thread/task or add your own guards. The alloc wrappers mirror your libc’s behavior.

---

## “Hello, AML” (all three in one place)

```c
#include "a-memory-library/aml_alloc.h"
#include "a-memory-library/aml_pool.h"
#include "a-memory-library/aml_buffer.h"

int main(void) {
#ifdef _AML_DEBUG_
  // Optional: write periodic snapshots of live allocations for debugging
  aml_alloc_log("allocations.log");
#endif

  // 1) Scratch arena for this unit of work
  aml_pool_t *scratch = aml_pool_init(32 << 10); // 32 KB

  // 2) A buffer that uses the pool's lifetime
  aml_buffer_t *b = aml_buffer_pool_init(scratch, 256);
  aml_buffer_appendf(b, "user:%s id:%d", "alice", 42);

  // Data is always NUL-terminated
  puts(aml_buffer_data(b));

  // 3) Need an owned copy? Detach into heap (if buffer was heap-backed).
  // For pool-backed buffers, detach returns pool memory (do not free it).

  // 4) Done with the batch: one call reclaims everything from the pool
  aml_pool_destroy(scratch);

  return 0;
}
```

---

## Getting started

### Dependencies

* C toolchain with C standard headers.
* [A cmake library](https://github.com/knode-ai-open-source/a-cmake-library) for the build system.

### Build & install

```bash
git clone https://github.com/knode-ai-open-source/a-memory-library.git
cd a-memory-library

mkdir -p build && cd build
cmake ..
make
make install
```

To enable debug diagnostics, build your code with `-D_AML_DEBUG_` (and recompile AML).

---

## Pick the right tool

* Use **`aml_pool`** when many allocations share a lifetime. Clear/destroy once; no per‑allocation frees.
* Use **`aml_buffer`** when you want a contiguous, growing byte/string buffer (e.g., response builders, encoders). It can live on the heap or inside a pool.
* Keep **`aml_alloc`** in your includes so you can switch on debug tracking without touching call sites.

---

## Learn more

* `aml_alloc`: features, behavior matrix (release vs debug), examples → **[`README.aml_alloc.md`](README.aml_alloc.md)**
* `aml_pool`: API surface, patterns (markers, sub‑pools), Base64/split helpers → **[`README.aml_pool.md`](README.aml_pool.md)**
* `aml_buffer`: invariants (always NUL‑terminated), alignment guarantees, detach semantics → **[`README.aml_buffer.md`](README.aml_buffer.md)**

---

## License

SPDX-License-Identifier: Apache-2.0
© 2019–2025 Andy Curtis

If something feels rough, open an issue or read the small sources — they’re meant to be understood.
