# README.aml\_pool.md — fast arena/region allocator for C

`aml_pool` is a tiny, **zero‑overhead** arena (a.k.a. region) allocator. It gives you `malloc`/`calloc`/`strdup`‑like functions plus a grab‑bag of helpers (aligned alloc, bulk dup, string splitting, Base64, pointer‑array duplication), but with one big difference:

> You don’t free individual allocations. You **clear** or **destroy** the pool to reclaim memory.

That trade‑off buys you very predictable performance, fewer syscalls, and dramatically simpler lifetime management for “allocate a bunch → use → discard” workflows.

---

## Quick start

```c
#include "a-memory-library/aml_pool.h"

int main(void) {
  // 1) Create a pool with an initial capacity (bytes)
  aml_pool_t *p = aml_pool_init(1024);

  // 2) Allocate like you would from malloc/calloc/strdup
  int   *a = (int*)   aml_pool_calloc(p, 10, sizeof(int));  // zeroed
  char  *s =          aml_pool_strdup(p, "hello, pool!");
  void  *raw =        aml_pool_alloc(p, 128);               // aligned to word
  void  *u   =        aml_pool_ualloc(p, 17);               // unaligned
  void  *simd =       aml_pool_aalloc(p, 64, 256);          // 64‑byte alignment

  // 3) Clear when you’re done with “a batch”
  aml_pool_clear(p);  // all pointers above become invalid!

  // 4) Destroy when finished with the pool itself
  aml_pool_destroy(p);
}
```

### Save/restore “stack‑like” usage

```c
aml_pool_marker_t m;
aml_pool_save(p, &m);

// allocate lots of scratch here…
char *tmp = (char*)aml_pool_alloc(p, 4096);

// rollback to the marker (drops allocations after save)
aml_pool_restore(p, &m);
```

### Sub‑pools (lightweight handles from a parent)

```c
aml_pool_t *root = aml_pool_init(1 << 20);   // big arena
aml_pool_t *sub  = aml_pool_pool_init(root, 16 << 10); // sub‑pool handle

char *s = (char*)aml_pool_alloc(sub, 1024);

// sub‑pool clear/destroy do NOT return memory to the parent; prefer sizing the
// sub‑pool correctly or use markers (save/restore) on the root when you need reuse.
aml_pool_destroy(sub);
aml_pool_destroy(root);
```

---

## What you get

### Creation & lifecycle

* `aml_pool_init(size_t size)` – create a pool with `size` bytes for the first block.
* `aml_pool_pool_init(aml_pool_t *parent, size_t size)` – a pool **backed by another pool** (see caveats below).
* `aml_pool_clear(aml_pool_t *p)` – invalidate all outstanding pointers and make memory reusable; frees extra blocks when the pool is **heap‑backed**.
* `aml_pool_destroy(aml_pool_t *p)` – destroy the pool; frees everything for heap‑backed pools.

### Allocation family

* `aml_pool_alloc(p, len)` – word‑aligned uninitialized bytes.
* `aml_pool_ualloc(p, len)` – **unaligned** uninitialized bytes.
* `aml_pool_zalloc(p, len)` / `aml_pool_calloc(p, n, size)` – zero‑initialized.
* `aml_pool_aalloc(p, alignment, len)` – power‑of‑two alignment (e.g. 64 for SIMD).
* `aml_pool_min_max_alloc(p, &rlen, min, max)` – returns at least `min` bytes and up to `max` in one shot (great for “fill as much as fits”).

### String & data helpers

* `aml_pool_strdup`, `aml_pool_strndup`, `aml_pool_strdupf`, `aml_pool_strdupvf`
* `aml_pool_dup` (aligned), `aml_pool_udup` (unaligned; appends a `'\0'` sentinel)
* Pointer‑array duplication:

    * `aml_pool_strdupa(pool, arr)` – deep‑copy a **NULL‑terminated** array of strings and the array itself.
    * `aml_pool_strdupan(pool, arr, n)` – deep‑copy **first `n` pointers**, **preserving NULLs inside**; array is terminated with an extra `NULL`.
    * `aml_pool_strdupa2(pool, arr)` – copy the **pointer array only** (no string duplication), up to the first `NULL`.

### Split helpers (tokenization)

* `aml_pool_split(p, &n, delim, s)` – splits into `n` tokens; **keeps empty tokens**; returns `char**` ending with `NULL`.
* `aml_pool_split2(p, &n, delim, s)` – same but **drops empty tokens**.
* `*_f` variants take a `printf`‑style format.
* `*_with_escape` versions honor an escape char (e.g. `\,` keeps comma).
* `*_with_escape2` also **drops empties**.

### Base64 utilities

* `aml_pool_base64_encode(p, data, len)` → pool‑owned null‑terminated Base64 text.
* `aml_pool_base64_decode(p, &out_len, b64)` → pool‑owned bytes; `out_len` set.

### Introspection

* `aml_pool_used(p)` – pool’s **own footprint** (bytes the pool has obtained from the underlying allocator across all blocks + header).
* `aml_pool_size(p)` – **immediately available bytes** remaining across the current active block(s) (capacity you can still allocate without growing).

> In debug builds (`_AML_DEBUG_`), the pool also tracks `cur_size` and the peak `max_size` internally for diagnostics.

---

## Behavior & performance

* **Amortized O(1) allocation.** The pool advances a bump pointer. If a block fills, it grows by at least `minimum_growth_size` (default = initial size). Set your own via `aml_pool_set_minimum_growth_size`.
* **Alignment:**

    * `aml_pool_alloc` returns pointers aligned to `sizeof(size_t)`.
    * `aml_pool_ualloc` is byte‑aligned (no guarantees).
    * `aml_pool_aalloc` enforces a **power‑of‑two** alignment (e.g. 16, 32, 64).
* **Thread safety:** not thread‑safe. Typical usage is **one pool per thread / task**.
* **No per‑allocation free.** Clearing/destroying invalidates *all* pointers allocated from the pool (and any string tokens returned by split helpers, etc.).
* **Sub‑pools caveat:** A sub‑pool allocates its memory from the parent. Clearing or destroying the sub‑pool **does not return** memory to the parent; it only resets the sub‑pool’s own cursors. Prefer markers on the parent when you want to reclaim.

---

## Common patterns

### Per‑request arena

```c
void handle_request(...) {
  aml_pool_t *scratch = aml_pool_init(32 << 10); // 32 KB
  // parse, build, serialize using scratch
  aml_pool_destroy(scratch); // one call; no leaks
}
```

### Reusable scratch with markers

```c
aml_pool_t *scratch = aml_pool_init(128 << 10);

for (...) {
  aml_pool_marker_t m;
  aml_pool_save(scratch, &m);

  // allocate lots of transient stuff here

  aml_pool_restore(scratch, &m); // cheap rollback
}

aml_pool_destroy(scratch);
```

### SIMD‑friendly aligned buffers

```c
float *v = (float*)aml_pool_aalloc(p, 64, 1024 * sizeof(float));
// v is 64‑byte aligned; safe for AVX512 loads
```

### Deep‑copy a string vector

```c
char *argv0[] = {"a", "bb", NULL};
char **dup = aml_pool_strdupa(p, argv0);
// dup[0]="a", dup[1]="bb", dup[2]=NULL; strings live in the pool
```

### Duplicate first N entries preserving NULL holes

```c
char *mixed[] = {"A", NULL, "B", NULL, "C", NULL};
char **d = aml_pool_strdupan(p, mixed, 6);
// d[0]="A", d[1]=NULL, d[2]="B", d[3]=NULL, d[4]="C", d[5]=NULL, d[6]=NULL
```

### Tokenize with escapes

```c
size_t n=0;
char **tok = aml_pool_split_with_escape(p, &n, ',', '\\', "a\\,b,c\\\\,d\\,\\,e");
// tok: ["a,b", "c\\", "d,,e"], tok[n]=NULL
```

### Base64

```c
const unsigned char blob[] = {1,2,3,4};
char *b64 = aml_pool_base64_encode(p, blob, sizeof blob);
size_t out = 0;
unsigned char *roundtrip = aml_pool_base64_decode(p, &out, b64);
```

---

## Gotchas & tips

* **Don’t keep pool pointers across `aml_pool_clear`/`aml_pool_destroy`.** That includes strings returned by `*_strdup*` and tokens returned by `*_split*`.
* **Choose a reasonable initial size.** If the pool grows frequently, you’ll see more block allocations. Use `aml_pool_set_minimum_growth_size` to tune growth.
* **Prefer markers (`save`/`restore`) over sub‑pools** when you need to repeatedly reuse the same memory region. Sub‑pools are just independent cursors; they don’t release memory back to the parent.
* `aml_pool_udup` appends a `'\0'` after the copied bytes — handy when duplicating binary data you’ll sometimes treat as a string.
* Split functions **duplicate** the input into the pool and then rewrite it in place (they replace delimiters with `'\0'`). The returned pointers point **into that pool copy**.
* `aml_pool_aalloc`: alignment must be a non‑zero power of two. In debug builds, invalid alignments abort.

---

## Memory model (how it works)

Internally the pool maintains a linked list of blocks. Each block tracks:

```c
typedef struct aml_pool_node_s {
  char *endp;                      // end of this block
  struct aml_pool_node_s *prev;    // previous block
} aml_pool_node_t;

struct aml_pool_s {
#ifdef _AML_DEBUG_
  // instrumentation: initial_size, cur_size, max_size
#endif
  aml_pool_node_t *current;        // active block
  char *curp;                      // bump pointer within current block
  size_t minimum_growth_size;      // lower bound for new block size
  size_t size;                     // internal accounting (see aml_pool_size)
  size_t used;                     // pool’s own footprint (see aml_pool_used)
  aml_pool_t *pool;                // non-NULL => this is a sub‑pool
};
```

Allocations bump `curp`. If there isn’t room, the pool grabs a new block (≥ `minimum_growth_size`), links it, and continues. Clearing frees extra blocks for heap‑backed pools and resets `curp` to the first block.

---

## Debug builds

If you compile with `_AML_DEBUG_`:

* `aml_pool_init`/`aml_pool_pool_init` capture the call‑site (for allocator diagnostics).
* The pool tracks `cur_size` and the high‑water mark `max_size`.
* Some APIs add extra checks (e.g., `aalloc` alignment).

This pairs well with the debug allocator in `aml_alloc` if you enable it across the project.

---

## FAQ

**Q: Why arenas instead of malloc/free?**
A: When your data has a common lifetime, arenas turn many tiny allocations into pointer bumping and a single `clear()`. It’s faster and simpler.

**Q: Can I free one thing from the pool?**
A: No. If you need that, keep a separate heap allocation for that object, or use a marker to pop back to a known point.

**Q: Do sub‑pools give memory back to the parent?**
A: No. They’re convenient independent cursors. To *reclaim* in the parent, use markers on the **parent** pool.

**Q: Is it thread‑safe?**
A: No. Give each thread/task its own pool or add your own synchronization.

---

## Testing

The test suite exercises:

* aligned/unaligned/zeroed allocations and growth policy
* markers (save/restore) and `clear()` behavior
* string helpers (`strdup*`, `*vf`, `strndup`)
* `min_max_alloc`, `aalloc` alignment
* string split family (with/without escapes, with/without empties)
* pointer‑array duplication (`strdupa`, `strdupan`, `strdupa2`) including internal `NULL`s
* Base64 encode/decode round‑trip
* sub‑pool lifecycle

Copy the patterns into your own project to validate assumptions.

---

## License

SPDX-License-Identifier: Apache-2.0
Copyright © 2019‑2025 Andy Curtis

If something feels rough or surprising, open an issue or peek at the implementation (`include/a-memory-library/impl/aml_pool.h` and `src/aml_pool.c`). It’s small and meant to be read.
