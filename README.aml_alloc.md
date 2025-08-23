# README.aml\_alloc.md — tiny allocation wrappers with **debug‑time** guard rails

`aml_alloc` gives you a small, convenient layer over the C runtime allocator plus a **diagnostic mode** that helps you catch leaks, double frees, and bad pointers — without paying overhead in release builds.

* **Release builds (`_AML_DEBUG_` not defined):** macros map directly to the C library (`malloc/calloc/realloc/free/strdup`). No tracking, no threads, no surprises.
* **Debug builds (`_AML_DEBUG_` defined):** every allocation is tracked with call‑site info, misuse is detected (double free, freeing foreign pointers), and you can dump live allocations or write periodic snapshots to a log.

It’s designed to be drop‑in: include the header, keep your code readable, and turn on debug tracking when you need to chase a problem.

---

# Quick start

```c
#include "a-memory-library/aml_alloc.h"

int main(void) {
#ifdef _AML_DEBUG_
  // Optional: write a snapshot of outstanding allocations every ~15s.
  // Call once early; the background thread stops at process exit.
  aml_alloc_log("allocations.log");
#endif

  char *s  = aml_strdup("hello");
  char *s2 = aml_strdupf("val=%d %s", 42, "ok");

  void *p  = aml_malloc(256);
  p = aml_realloc(p, 512);

  aml_free(s2);
  aml_free(s);

  // Safe: aml_free(NULL) is a no-op
  aml_free(NULL);

#ifdef _AML_DEBUG_
  // Print a summary of current live allocations to stderr (or any FILE*).
  aml_dump(stderr);
#endif
}
```

---

# What you get

## Core macros (always available)

These names are what you call in your code:

* `aml_malloc(len)` / `aml_zalloc(len)` / `aml_calloc(num, size)`
* `aml_realloc(ptr, len)` / `aml_free(ptr)`
* `aml_strdup(s)` / `aml_strdupf(fmt, ...)` / `aml_strdupvf(fmt, va_list)`
* `aml_dup(ptr, len)` – duplicate raw bytes
* **Array‑of‑strings helpers** (see behavior below):

    * `aml_strdupa(char **arr)` – deep‑copy a NULL‑terminated `char**` and all strings
    * `aml_strdupan(char **arr, size_t n)` – deep‑copy the **first `n` elements**, **preserving `NULL`s** inside; appends a trailing `NULL`
    * `aml_strdupa2(char **arr)` – copy **the pointer array only** (no string duplication)

> In **release** builds these expand to the C library (plus small helpers).
> In **debug** builds they capture the call‑site and route through the debug allocator.

## Debug‑only extras

* `aml_dump(FILE *out)`
  Print a summary of **currently live** allocations, including:

    * total bytes and allocation count
    * per‑allocation call‑site (file\:line \[tag])

* `aml_alloc_log(const char *filename)`
  Start a background thread that writes a timestamped snapshot of live allocations to `filename` about every 15 seconds. Simple log rotation is applied (`.1`, `.2`, … older snapshots).
  **Tip:** call it **once** early; it stops automatically at shutdown.

> Under the hood, `aml_alloc` installs a process‑lifetime allocator object in debug builds (via constructor/destructor attributes) so you don’t need to “init” or “shutdown” anything in user code.

---

# Behavior and guarantees

## Release vs Debug (what changes)

| Aspect        | Release (default)        | Debug (`_AML_DEBUG_`)                                                 |
| ------------- | ------------------------ | --------------------------------------------------------------------- |
| API surface   | same names               | same names + `aml_dump`, `aml_alloc_log`                              |
| Overhead      | none (direct libc calls) | per‑allocation book‑keeping, a mutex, optional logging thread         |
| Safety checks | libc‑only                | double‑free detection, invalid pointer detection, useful crash output |

### Misuse diagnostics (debug)

* **Double free / foreign pointer:** the allocator prints where the bad `aml_free` came from; it also tries to point out the *closest known allocation* to help spot off‑by‑one or interior frees, then aborts.
* **Out‑of‑memory:** the site that attempted the allocation is printed before abort.

### Zero‑length allocations

* `aml_malloc(0)`, `aml_zalloc(0)`, `aml_calloc(0, x)`, `aml_realloc(NULL, 0)`, `aml_dup(..., 0)`:

    * **Debug:** return `NULL` (explicitly).
    * **Release:** behavior follows your libc (may return `NULL` or a unique pointer).
      **Guideline:** treat length‑0 as “no allocation expected”.

### `aml_strdupa` family (deep copies of argv‑style arrays)

* `aml_strdupa(arr)` copies **each string** and the **pointer array**, stopping at the first `NULL`. Result is terminated with `NULL`.
* `aml_strdupan(arr, n)` copies the **first `n` entries**:

    * Each non‑NULL string is duplicated.
    * Any **NULL entries are preserved** as `NULL` in the result.
    * The result has an extra `NULL` terminator at index `n`.
* `aml_strdupa2(arr)` only duplicates the **pointer array** (shallow copy), up to the terminating `NULL`.

---

# Practical snippets

## Format helpers

```c
static char *vdup(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char *s = aml_strdupvf(fmt, ap);
  va_end(ap);
  return s;
}
```

## Deep copy an argv‑style array (preserving internal NULLs)

```c
char *in[] = {"A", NULL, "B", NULL, "C", NULL};
char **out = aml_strdupan(in, 6);
// out[0]="A", out[1]=NULL, out[2]="B", out[3]=NULL, out[4]="C", out[5]=NULL, out[6]=NULL
```

## Dump live allocations on failure paths (debug only)

```c
if (error) {
#ifdef _AML_DEBUG_
  fprintf(stderr, "Error happened; dumping live allocations:\n");
  aml_dump(stderr);
#endif
  return -1;
}
```

---

# How the debug allocator works (high‑level)

* Every `aml_*alloc` call records a node with:

    * call‑site string (`__FILE__:__LINE__ [api]`),
    * requested length (negative lengths are reserved for AML objects that attach a custom dump header),
    * links into a global doubly‑linked list.
* `aml_free` removes the node; `aml_realloc` allocates a new block, copies `min(old,new)` bytes, frees the old.
* `aml_dump` walks the list and pretty‑prints; if an object advertises a custom dump routine (e.g., other AML types in debug mode), that is invoked to show type‑specific details.
* `aml_alloc_log` writes periodic snapshots and rotates simple numbered copies.

All of this is **completely inert** in release builds.

---

# FAQ

**Q: Do I have to initialize anything?**
A: No user‑init is required. In debug builds the allocator is set up automatically (constructor), and torn down at process exit (destructor).

**Q: Can I call `aml_alloc_log` more than once? Change the filename?**
A: Call it **once** early if you want logging. Changing the filename mid‑run isn’t supported.

**Q: Is this thread‑safe?**
A: The internal list is protected by a mutex in debug builds. The public API mirrors libc threading behavior.

**Q: What about performance?**
A: Release builds are just libc. Debug builds add book‑keeping for safety/diagnostics; keep it for tests and bug hunts.

**Q: Why do some AML types show rich details in dumps?**
A: In debug, AML objects can attach a small “dump header” so `aml_dump` knows how to print them (e.g., buffers/pools show sizes).

---

# Testing notes

The project’s tests cover:

* malloc/zalloc/calloc/realloc/free happy paths
* `strdup`/`strdupf`/`strdupvf` and binary `dup`
* the `strdupa` family (including arrays with internal `NULL`s and partial copies)
* realloc grow/shrink, NULL‑realloc behavior
* large allocation success, ping‑pong / stress scenarios
* debug‑mode only behaviors (zero‑length returns, dump/log safety)

If you’re integrating into your codebase, copy those patterns, and sprinkle in `aml_dump(stderr)` at key failure points during development.

---

# License

SPDX-License-Identifier: Apache-2.0
Copyright © 2019‑2025 Andy Curtis

Questions or rough edges? Open an issue or skim the tiny implementation. It’s meant to be read.
