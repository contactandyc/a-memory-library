# README.aml\_buffer.md — auto‑growing byte/string buffer (heap or pool‑backed)

`aml_buffer` is a tiny, fast, **contiguous** buffer with a friendly API for both text and binary data. It handles growth automatically, keeps the content **NUL‑terminated** at all times, and integrates with `aml_pool` when you want arena‑style lifetimes.

* **Set** or **append** bytes, chars, and formatted strings.
* Reserve writable space (aligned or unaligned), then fill it yourself.
* **Detach** the underlying storage when you need to “hand it off”.
* Debug builds piggyback on `aml_alloc` to help you find leaks/misuse.

---

## Quick start

```c
#include "a-memory-library/aml_buffer.h"

int main(void) {
  aml_buffer_t *b = aml_buffer_init(0);      // heap-backed; starts tiny

  aml_buffer_appends(b, "hello");
  aml_buffer_appendc(b, ' ');
  aml_buffer_appendf(b, "world %d", 42);

  // Always NUL-terminated: data()[length()] == '\0'
  puts(aml_buffer_data(b));                   // "hello world 42"

  // Transfer ownership of the bytes
  size_t len = 0;
  char *owned = aml_buffer_detach(b, &len);   // caller now owns 'owned'
  // ... use 'owned' ...
  aml_free(owned);

  aml_buffer_destroy(b);                      // frees the handle
}
```

### Pool-backed variant

```c
aml_pool_t   *pool = aml_pool_init(1024);
aml_buffer_t *bp   = aml_buffer_pool_init(pool, 64);

aml_buffer_appends(bp, "pool memory");
char *p = aml_buffer_data(bp);

// Detach returns pool memory; do NOT free it. It becomes invalid when the pool is destroyed.
size_t L=0; char *aliased = aml_buffer_detach(bp, &L);

// Tear down the pool (invalidates 'aliased')
aml_pool_destroy(pool);
```

> **Ownership rule:**
> – Heap‑backed: `detach` returns a heap pointer; free with `aml_free`.
> – Pool‑backed: `detach` returns **pool** memory; **do not free**; it dies with the pool.

---

## API surface

You include **`aml_buffer.h`**; most functions are `static inline` with a small non‑inline core in `src/aml_buffer.c`.

### Create / destroy

* `aml_buffer_t *aml_buffer_init(size_t initial_size);`
* `aml_buffer_t *aml_buffer_pool_init(aml_pool_t *pool, size_t initial_size);`
* `void aml_buffer_destroy(aml_buffer_t *h);`
  *No action for pool‑backed buffers; lifetime is tied to the pool.*

### Introspection

* `char  *aml_buffer_data(aml_buffer_t *h);`       → contiguous bytes, **always NUL‑terminated**
* `size_t aml_buffer_length(aml_buffer_t *h);`
* `char  *aml_buffer_end(aml_buffer_t *h);`        → `data() + length()`

### Set (replace current content)

* `void aml_buffer_set (aml_buffer_t*, const void *data, size_t len);`
* `void aml_buffer_sets(aml_buffer_t*, const char *s);`
* `void aml_buffer_setc(aml_buffer_t*, char ch);`
* `void aml_buffer_setn(aml_buffer_t*, char ch, ssize_t n);`    *(n ≤ 0 is a no‑op)*
* `void aml_buffer_setvf(aml_buffer_t*, const char *fmt, va_list ap);`
* `void aml_buffer_setf (aml_buffer_t*, const char *fmt, ...);`

### Append (add to current content)

* `void aml_buffer_append (aml_buffer_t*, const void *data, size_t len);`
* `void aml_buffer_appends(aml_buffer_t*, const char *s);`
* `void aml_buffer_appendsz(aml_buffer_t*, const char *s);`     *(appends `s` **and** its `'\0'`)*
* `void aml_buffer_appendc(aml_buffer_t*, char ch);`
* `void aml_buffer_appendn(aml_buffer_t*, char ch, ssize_t n);`
* `void aml_buffer_appendvf(aml_buffer_t*, const char *fmt, va_list ap);`
* `void aml_buffer_appendf (aml_buffer_t*, const char *fmt, ...);`

### Reserve/resize (manual writes)

* `void *aml_buffer_resize      (aml_buffer_t*, size_t new_len);`
  Grows as needed and **preserves** existing bytes.
* `void *aml_buffer_append_alloc(aml_buffer_t*, size_t len);`
  Appends **len** bytes and returns a writable span; **write‑position is 8‑byte aligned**.
  *Good for appending structs/integers safely.*
* `void *aml_buffer_append_ualloc(aml_buffer_t*, size_t len);`
  Like above, but **no alignment** guarantees.
* `void *aml_buffer_alloc       (aml_buffer_t*, size_t len);`
  Resizes to exactly **len** but **does not preserve** previous contents (may reallocate and clobber).

### Maintenance

* `void aml_buffer_clear(aml_buffer_t*);`         → set length to 0; capacity unchanged
* `void aml_buffer_reset(aml_buffer_t*, size_t max_size);`
  If heap‑backed and capacity is bigger than `max_size`, shrinks the backing storage; length becomes 0.
  *(Pool‑backed: just clears length.)*
* `void *aml_buffer_shrink_by(aml_buffer_t*, size_t n);`
  Truncates by **n** bytes (or clears if `n ≥ length`).

### Transfer

* `char *aml_buffer_detach(aml_buffer_t*, size_t *length_out);`
  Returns the current backing pointer and **re‑initializes** the handle to an empty, valid state so it can be reused. See ownership rule above.

---

## Behavior & notes

* **NUL‑termination invariant:** after any operation, `data()[length()] == '\0'`.
  This makes the buffer usable as a string without extra steps, even when you’re appending binary.
* **Alignment:** `aml_buffer_append_alloc` aligns the write position to **8 bytes** before reserving (helpful for packing PODs). Use `append_ualloc` if you don’t care.
* **Growth policy:** capacity grows geometrically (implementation detail); do not rely on exact sizes.
* **Reset vs clear:**

    * `clear()` keeps capacity; `reset(max)` can shrink heap capacity if it overshot.
    * On pool‑backed buffers, `reset()` behaves like `clear()` (no shrink).
* **Detach & reuse:** after `detach()`, the buffer is safe to keep using; you don’t need to create a new `aml_buffer_t`.

---

## Examples

### Append formatted bits, then binary structs

```c
typedef struct { uint32_t id; uint16_t flags; } Rec;

aml_buffer_t *b = aml_buffer_init(64);
aml_buffer_appendf(b, "hdr:%s\n", "v1");

// aligned space for a record
Rec *r = (Rec*)aml_buffer_append_alloc(b, sizeof(Rec));
r->id = 1001u; r->flags = 0x7;

// more text
aml_buffer_appendf(b, "\nsize=%zu", aml_buffer_length(b));
```

### Use as a scratch string with periodic shrinking

```c
aml_buffer_t *b = aml_buffer_init(0);
for (int i = 0; i < 1000; ++i) {
  aml_buffer_clear(b);
  aml_buffer_appendf(b, "row=%d, payload=%s", i, "...");
  // occasionally trim back oversized capacity
  if ((i % 128) == 0) aml_buffer_reset(b, 16 * 1024);
}
aml_buffer_destroy(b);
```

### Pool‑backed lifetime

```c
aml_pool_t *pool = aml_pool_init(4096);
aml_buffer_t *b   = aml_buffer_pool_init(pool, 256);
aml_buffer_appendf(b, "Name: %s", "Alice");

// Detach is an alias into the pool; don't free it.
size_t L; char *s = aml_buffer_detach(b, &L);

// Destroying the pool invalidates both b’s storage and s.
aml_pool_destroy(pool);
```

---

## Debug mode

When `_AML_DEBUG_` is defined:

* Buffer allocations route through `aml_alloc` (with file/line tags).
* Other AML types can add extra dump info; buffer internals track peak length, etc.
* You can call `aml_dump(stderr)` (from `aml_alloc`) to see live allocations, including buffers.

Release builds avoid all tracking and call straight into `malloc`/`free`.

---

## Common pitfalls

* **Do not free** memory returned by `detach()` if the buffer was **pool‑backed**. It belongs to the pool.
  *(Heap‑backed is fine; free with `aml_free`.)*
* `aml_buffer_alloc()` **does not preserve** previous content. Use `resize()` or `append_*` if you need preservation.
* `appendn(..., n ≤ 0)` is a no‑op by design.

---

## Complexity

Amortized **O(1)** appends; all operations are bounded by the number of growth reallocations. Access to `data()`/`end()`/`length()` is O(1).

---

## License

SPDX-License-Identifier: Apache-2.0
Copyright © 2019–2025 Andy Curtis

If something feels off, peek at the single small implementation or open an issue. The code is meant to be read.
