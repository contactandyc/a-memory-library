/*
Copyright 2019 Andy Curtis

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "a-memory-library/aml_alloc.h"
#include "a-memory-library/aml_pool.h"
#include <stdlib.h>
#include <stdint.h>

size_t aml_pool_size(aml_pool_t *h) {
  return h->size + (h->current->endp - h->curp);
}

size_t aml_pool_used(aml_pool_t *h) { return h->used; }

void aml_pool_set_minimum_growth_size(aml_pool_t *h, size_t size) {
  if (size == 0)
    abort(); /* this doesn't make sense */
  h->minimum_growth_size = size;
}

#ifdef _AML_DEBUG_
static void dump_pool(FILE *out, const char *caller, void *p, size_t length) {
  aml_pool_t *pool = (aml_pool_t *)p;
  fprintf(out, "%s size: %lu, max_size: %lu, initial_size: %lu used: %lu ",
          caller, pool->cur_size, pool->max_size, pool->initial_size,
          pool->used);
}

aml_pool_t *_aml_pool_init(size_t initial_size, const char *caller) {
#else
aml_pool_t *_aml_pool_init(size_t initial_size) {
#endif
  if (initial_size == 0)
    abort(); /* this doesn't make any sense */
  /* round initial_size up to be properly aligned */
  initial_size += ((sizeof(size_t) - (initial_size & (sizeof(size_t) - 1))) &
                   (sizeof(size_t) - 1));

  /* Allocate the aml_pool_t structure, the first node, and the memory in one
   call.  This keeps the memory in close proximity which is better for the CPU
   cache.  It also makes it so that when we destroy the handle, we only need to
   make one call to free for the handle, the first block, and the initial size.
  */

  /* If the initial_size is an even multiple of 4096, then reduce the block size
   so that the actual memory allocated via the system malloc is 4096 bytes. */
  size_t block_size = initial_size;
  if ((block_size & 4096) == 0)
    block_size -= (sizeof(aml_pool_t) + sizeof(aml_pool_node_t));

  aml_pool_t *h;
#ifdef _AML_DEBUG_
  h = (aml_pool_t *)_aml_malloc_d(
      caller, block_size + sizeof(aml_pool_t) + sizeof(aml_pool_node_t),
      true);
  memset(h, 0, sizeof(aml_pool_t) + sizeof(aml_pool_node_t));
  h->dump.dump = dump_pool;
  h->initial_size = initial_size;
  h->cur_size = 0;
  h->max_size = 0;
#else
  h = (aml_pool_t *)aml_malloc(block_size + sizeof(aml_pool_t) +
                             sizeof(aml_pool_node_t));
  memset(h, 0, sizeof(aml_pool_t) + sizeof(aml_pool_node_t));
#endif
  if (!h) /* what else might we do? */
    abort();
  h->used = initial_size + sizeof(aml_pool_t) + sizeof(aml_pool_node_t);
  h->size = 0;
  h->pool = NULL;
  h->current = (aml_pool_node_t *)(h + 1);
  h->curp = (char *)(h->current + 1);
  h->current->endp = h->curp + block_size;
  h->current->prev = NULL;

  aml_pool_set_minimum_growth_size(h, initial_size);
  return h;
}



aml_pool_t *aml_pool_pool_init(aml_pool_t *pool, size_t initial_size) {
  if (initial_size == 0)
    abort(); /* this doesn't make any sense */
  /* round initial_size up to be properly aligned */
  initial_size += ((sizeof(size_t) - (initial_size & (sizeof(size_t) - 1))) &
                   (sizeof(size_t) - 1));
  size_t block_size = initial_size;

  aml_pool_t *h;
  h = (aml_pool_t *)aml_pool_alloc(pool, block_size + sizeof(aml_pool_t) +
                                 sizeof(aml_pool_node_t));
  if (!h) /* what else might we do? */
    abort();
  h->used = initial_size + sizeof(aml_pool_t) + sizeof(aml_pool_node_t);
  h->size = 0;
  h->current = (aml_pool_node_t *)(h + 1);
  h->curp = (char *)(h->current + 1);
  h->current->endp = h->curp + block_size;
  h->current->prev = NULL;
  h->pool = pool;
  aml_pool_set_minimum_growth_size(h, initial_size);
  return h;
}


void aml_pool_clear(aml_pool_t *h) {
  /* remove the extra blocks (the ones where prev != NULL) */
  aml_pool_node_t *prev = h->current->prev;
  while (prev) {
    if(!h->pool)
      aml_free(h->current);
    h->current = prev;
    prev = prev->prev;
  }

  /* reset curp to the beginning */
  h->curp = (char *)(h->current + 1);

  /* reset size and used */
  h->size = 0;
#ifdef _AML_DEBUG_
  h->cur_size = 0;
#endif
  h->used =
      (h->current->endp - h->curp) + sizeof(aml_pool_t) + sizeof(aml_pool_node_t);
}

void aml_pool_destroy(aml_pool_t *h) {
  /* pool_clear frees all of the memory from all of the extra nodes and only
    leaves the main block and main node allocated */
  aml_pool_clear(h);
  /* free the main block and the main node */
  if(!h->pool)
    aml_free(h);
}


void *aml_pool_aalloc(aml_pool_t *pool, size_t alignment, size_t size) {
#ifdef _AML_DEBUG_
    // Only check in debug mode
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        // Alignment must be a power of 2
        abort();
    }
#endif

    // Check if there's enough space in the current block for aligned allocation
    uintptr_t current = (uintptr_t)pool->curp;
    uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1);
    size_t padding = aligned - current;
    if (pool->curp + padding + size <= pool->current->endp) {
        pool->curp += padding;  // Adjust pointer to aligned address
        void *result = pool->curp;
        pool->curp += size;  // Reserve the requested size
        pool->used += padding + size;  // Update used size
#ifdef _AML_DEBUG_
        pool->cur_size += padding + size;
        if (pool->cur_size > pool->max_size) {
            pool->max_size = pool->cur_size;
        }
#endif
        return result;
    }

    // Not enough space in the current block, grow the pool and allocate aligned
    size_t total_size = size + alignment - 1;  // Account for alignment padding
    char *block = (char *)_aml_pool_alloc_grow(pool, total_size);
    aligned = ((uintptr_t)block + alignment - 1) & ~(alignment - 1);
    return (void *)aligned;
}


void *_aml_pool_alloc_grow(aml_pool_t *h, size_t len) {
  size_t block_size = len;
  if (block_size < h->minimum_growth_size)
    block_size = h->minimum_growth_size;
  aml_pool_node_t *block;
  if(!h->pool)
    block = (aml_pool_node_t *)aml_malloc(sizeof(aml_pool_node_t) + block_size);
  else
    block = (aml_pool_node_t *)aml_pool_alloc(h->pool, sizeof(aml_pool_node_t) + block_size);
  if (!block)
    abort();
  if (h->current->prev)
    h->size += (h->current->endp - h->curp);
  h->used += sizeof(aml_pool_node_t) + block_size;
  block->prev = h->current;
  h->current = block;
  char *r = (char *)(block + 1);
  block->endp = r + block_size;
  h->curp = r + len;
#ifdef _AML_DEBUG_
  h->cur_size += len;
  if (h->cur_size > h->max_size)
    h->max_size = h->cur_size;
#endif
  return r;
}

char *aml_pool_strdupvf(aml_pool_t *pool, const char *fmt, va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);
  size_t leftover = pool->current->endp - pool->curp;
  char *r = pool->curp;
  int n = vsnprintf(r, leftover, fmt, args_copy);
  if (n < 0)
    abort();
  va_end(args_copy);
  if (n < leftover) {
    pool->curp += n + 1;
#ifdef _AML_DEBUG_
    pool->cur_size += (n + 1);
    if (pool->cur_size > pool->max_size)
      pool->max_size = pool->cur_size;
#endif
    return r;
  }
  r = (char *)aml_pool_ualloc(pool, n + 1);
  va_copy(args_copy, args);
  int n2 = vsnprintf(r, n + 1, fmt, args_copy);
  if (n != n2)
    abort(); // should never happen!
  va_end(args_copy);
  return r;
}

static size_t count_bytes_in_array(char **a, size_t *n) {
  size_t len = sizeof(char *);
  size_t num = 1;
  while (*a) {
    len += strlen(*a) + sizeof(char *) + 1;
    num++;
    a++;
  }
  return len;
}

char **aml_pool_strdupa(aml_pool_t *pool, char **a) {
  if (!a)
    return NULL;

  size_t n = 0;
  size_t len = count_bytes_in_array(a, &n);
  char **r = (char **)aml_pool_alloc(pool, len);
  char *m = (char *)(r + n);
  char **rp = r;
  while (*a) {
    *rp++ = m;
    char *s = *a;
    while (*s)
      *m++ = *s++;
    *m++ = 0;
    a++;
  }
  *rp = NULL;
  return r;
}

static size_t count_bytes_in_arrayn(char **a, size_t num) {
  size_t len = (sizeof(char *) * (num + 1));
  for (size_t i = 0; i < num; i++) {
    len += strlen(*a) + 1;
    a++;
  }
  return len;
}

char **aml_pool_strdupan(aml_pool_t *pool, char **a, size_t n) {
  if (!a)
    return NULL;

  size_t len = count_bytes_in_arrayn(a, n);
  char **r = (char **)aml_pool_alloc(pool, len);
  char *m = (char *)(r + n + 1);
  char **rp = r;
  char **ae = a + n;
  while (a < ae) {
    *rp++ = m;
    char *s = *a;
    while (*s)
      *m++ = *s++;
    *m++ = 0;
    a++;
  }
  *rp = NULL;
  return r;
}

char **aml_pool_strdupa2(aml_pool_t *pool, char **a) {
  if (!a)
    return NULL;

  char **p = a;
  while (*p)
    p++;

  p++;
  return (char **)aml_pool_dup(pool, a, (p - a) * sizeof(char *));
}


char **_aml_pool_split(aml_pool_t *h, size_t *num_splits, char delim, char *s) {
  static char *nil = NULL;
  if (!s) {
    if (num_splits)
      *num_splits = 0;
    return &nil;
  }
  char *p = s;
  size_t num = 1;
  while (*p != 0) {
    if (*p == delim)
      num++;
    p++;
  }
  if (num_splits)
    *num_splits = num;
  char **r = (char **)aml_pool_alloc(h, sizeof(char *) * (num + 1));
  char **wr = r;
  *wr = s;
  wr++;
    // ,,, => "","","",""

  while (*s != 0) {
    if (*s == delim) {
      *s = 0;
      s++;
      *wr = s;
      wr++;
    } else
      s++;
  }
  *wr = NULL;
  return r;
}

char **aml_pool_split(aml_pool_t *h, size_t *num_splits, char delim,
                     const char *p) {
  return _aml_pool_split(h, num_splits, delim, p ? aml_pool_strdup(h, p) : NULL);
}

char **aml_pool_splitf(aml_pool_t *h, size_t *num_splits, char delim,
                      const char *p, ...) {
  va_list args;
  va_start(args, p);
  char *r = aml_pool_strdupvf(h, p, args);
  va_end(args);
  return _aml_pool_split(h, num_splits, delim, r);
}

char **_aml_pool_split2(aml_pool_t *h, size_t *num_splits, char delim, char *s) {
  size_t num_res = 0;
  char **res = _aml_pool_split(h, &num_res, delim, s);
  char **wp = res;
  char **p = res;
  char **ep = p+num_res;
  while(p < ep) {
    if(*p[0] != 0)
        *wp++ = *p;
    p++;
  }
  *num_splits = wp-res;
  *wp++ = NULL;
  return res;
}

char **aml_pool_split2(aml_pool_t *h, size_t *num_splits, char delim,
                      const char *p) {
  return _aml_pool_split2(h, num_splits, delim, p ? aml_pool_strdup(h, p) : NULL);
}

char **aml_pool_split2f(aml_pool_t *h, size_t *num_splits, char delim,
                       const char *p, ...) {
  va_list args;
  va_start(args, p);
  char *r = aml_pool_strdupvf(h, p, args);
  va_end(args);
  return _aml_pool_split2(h, num_splits, delim, r);
}

char **_aml_pool_split_with_escape(aml_pool_t *h, size_t *num_splits, char delim, char escape, const char *p) {
    if (!p) {
        if (num_splits)
            *num_splits = 0;
        static char *nil = NULL;
        return &nil;
    }

    // Duplicate the string into the pool for modification
    char *s = aml_pool_strdup(h, p);
    char *current = s;
    size_t count = 1; // At least one split by default
    int escape_next = 0; // Flag to handle escape sequences

    // First pass: Count splits while handling escape characters
    for (char *c = s; *c; ++c) {
        if (escape_next) {
            escape_next = 0; // Skip this character; it was escaped
            continue;
        }
        if (*c == escape) {
            escape_next = 1; // Escape the next character
            continue;
        }
        if (*c == delim) {
            ++count;
        }
    }

    // Allocate memory for the result array
    char **result = (char **)aml_pool_alloc(h, sizeof(char *) * (count + 1));

    size_t idx = 0;
    result[idx++] = current; // First segment starts at the beginning
    escape_next = 0;

    // Second pass: Split the string while removing escape characters
    for (char *c = s; *c; ++c) {
        if (escape_next) {
            escape_next = 0; // Keep the escaped character
            *current++ = *c;
            continue;
        }
        if (*c == escape) {
            escape_next = 1; // Mark next character for escaping
            continue;
        }
        if (*c == delim) {
            *current++ = '\0'; // Terminate the current segment
            result[idx++] = current; // Start a new segment
            continue;
        }
        *current++ = *c; // Copy character
    }

    *current = '\0'; // Null-terminate the final segment
    result[idx] = NULL; // Null-terminate the result array

    if (num_splits) {
        *num_splits = idx;
    }

    return result;
}

char **aml_pool_split_with_escape(aml_pool_t *h, size_t *num_splits, char delim, char escape,
                                  const char *p) {
  return _aml_pool_split_with_escape(h, num_splits, delim, escape, p ? aml_pool_strdup(h, p) : NULL);
}

char **aml_pool_split_with_escapef(aml_pool_t *h, size_t *num_splits, char delim, char escape,
                                   const char *p, ...) {
  va_list args;
  va_start(args, p);
  char *r = aml_pool_strdupvf(h, p, args);
  va_end(args);
  return _aml_pool_split_with_escape(h, num_splits, delim, escape, r);
}

char **_aml_pool_split_with_escape2(aml_pool_t *h, size_t *num_splits, char delim, char escape, char *s) {
  size_t num_res = 0;
  char **res = _aml_pool_split_with_escape(h, &num_res, delim, escape, s);
  char **wp = res;
  char **p = res;
  char **ep = p+num_res;
  while(p < ep) {
    if(*p[0] != 0)
        *wp++ = *p;
    p++;
  }
  *num_splits = wp-res;
  *wp++ = NULL;
  return res;
}

char **aml_pool_split_with_escape2(aml_pool_t *h, size_t *num_splits, char delim, char escape,
                                   const char *p) {
  return _aml_pool_split_with_escape2(h, num_splits, delim, escape, p ? aml_pool_strdup(h, p) : NULL);
}


char **aml_pool_split_with_escape2f(aml_pool_t *h, size_t *num_splits, char delim, char escape,
                                    const char *p, ...) {
  va_list args;
  va_start(args, p);
  char *r = aml_pool_strdupvf(h, p, args);
  va_end(args);
  return aml_pool_split_with_escape2(h, num_splits, delim, escape, r);
}
