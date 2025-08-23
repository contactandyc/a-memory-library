// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

/* IMPLEMENTATION FOLLOWS - API is above this line */

/* Because this object is called very frequently, some of the functionality is
  inlined. Inlining the structure can be helpful for other objects, particularly
  if they want to be able to take advantage of the remaining memory in a block
  in the way that pool_strdupf does. */

/* used internally */
void *_aml_pool_alloc_grow(aml_pool_t *h, size_t len);

// #ifndef _AML_USE_MALLOC_
// #define _AML_USE_MALLOC_
// #endif

typedef struct aml_pool_node_s {
  /* The aml_pool_node_s includes a block of memory just after it.  endp
    points to the end of that block of memory.
   */
  char *endp;

  /* this will be NULL if it is the first block. */
  struct aml_pool_node_s *prev;
} aml_pool_node_t;

struct aml_pool_s {
#ifdef _AML_DEBUG_
  aml_allocator_dump_t dump;
  /* The size of the initial block requires a second variable to be
     thread-safe. */
  size_t initial_size;
  /* The cur_size is needed because the aml_pool_size function isn't
     thread-safe. */
  size_t cur_size;
  /* Everytime the pool get's cleared, cur_size is reset */
  size_t max_size;
#endif

  /* A pointer to the current block associated with the pool.  If there is more
    than one block, the blocks are linked together via the prev pointer into
    a singly linked list. */
  aml_pool_node_t *current;

  /* A pointer into the current node where memory is available. */
  char *curp;

  /* This is used as an alternate size for new blocks beyond the initial
    block.  It will be initially set to the length of the first block and can
    later be modified. */
  size_t minimum_growth_size;

  /* the size doesn't consider the bytes that are used in the current block */
  size_t size;

  /* the total number of bytes allocated by the pool object */
  size_t used;

  /* if set, memory is allocated from this pool */
  aml_pool_t *pool;
};

static inline void *aml_pool_ualloc(aml_pool_t *h, size_t len) {
  char *r = h->curp;
  if (r + len < h->current->endp) {
    h->curp = r + len;
#ifdef _AML_DEBUG_
    h->cur_size += len;
    if (h->cur_size > h->max_size)
      h->max_size = h->cur_size;
#endif
    return r;
  }
  return _aml_pool_alloc_grow(h, len);
}

static inline void *aml_pool_min_max_alloc(aml_pool_t *h, size_t *rlen,
                                          size_t min_len, size_t len) {
  char *r =
      h->curp + ((sizeof(size_t) - ((size_t)(h->curp) & (sizeof(size_t) - 1))) &
                 (sizeof(size_t) - 1));
  if (r + len < h->current->endp) {
    h->curp = r + len;
#ifdef _AML_DEBUG_
    h->cur_size += len;
    if (h->cur_size > h->max_size)
      h->max_size = h->cur_size;
#endif
    *rlen = len;
    return r;
  }
  if (r + min_len < h->current->endp) {
    len = (h->current->endp - r) - 1;
    h->curp = r + len;
#ifdef _AML_DEBUG_
    h->cur_size += len;
    if (h->cur_size > h->max_size)
      h->max_size = h->cur_size;
#endif
    *rlen = len;
    return r;
  }
  *rlen = len;
  return _aml_pool_alloc_grow(h, len);
}

static inline void *aml_pool_alloc(aml_pool_t *h, size_t len) {
  size_t to_add = ((sizeof(size_t) - ((size_t)(h->curp) & (sizeof(size_t) - 1))) &
                                   (sizeof(size_t) - 1));
  char *r =
      h->curp + to_add;
  if (r + len < h->current->endp) {
    h->curp = r + len;
#ifdef _AML_DEBUG_
    h->cur_size += len;
    if (h->cur_size > h->max_size)
      h->max_size = h->cur_size;
#endif
    return r;
  }
  return _aml_pool_alloc_grow(h, len);
}

static inline void *aml_pool_zalloc(aml_pool_t *h, size_t len) {
  /* calloc will simply call the pool_alloc function and then zero the memory.
   */
  void *dest = aml_pool_alloc(h, len);
  if (len)
    memset(dest, 0, len);
  return dest;
}

static inline void *aml_pool_calloc(aml_pool_t *h, size_t num_items, size_t size) {
  /* calloc will simply call the pool_alloc function and then zero the memory.
   */
  return aml_pool_zalloc(h, num_items*size);
}

static inline void *aml_pool_udup(aml_pool_t *h, const void *data, size_t len) {
  /* dup will simply allocate enough bytes to hold the duplicated data,
    copy the data, and return the newly allocated memory which contains a copy
    of data. Because the data could need aligned, we will use aml_pool_alloc
    instead of aml_pool_ualloc */
  char *dest = (char *)aml_pool_ualloc(h, len + 1);
  if (len)
    memcpy(dest, data, len);
  dest[len] = 0;
  return dest;
}

static inline char *aml_pool_strdup(aml_pool_t *h, const char *p) {
  /* strdup will simply allocate enough bytes to hold the duplicated string,
    copy the string, and return the newly allocated string. */
  size_t len = strlen(p);
  return (char *)aml_pool_udup(h, p, len);
}

static inline char *aml_pool_strndup(aml_pool_t *h, const char *p,
                                    size_t length) {
  /* strdup will simply allocate enough bytes to hold the duplicated string,
    copy the string, and return the newly allocated string. */
  size_t len = strlen(p);
  if (len > length)
    len = length;
  return (char *)aml_pool_udup(h, p, len);
}

static inline void *aml_pool_dup(aml_pool_t *h, const void *data, size_t len) {
  /* dup will simply allocate enough bytes to hold the duplicated data,
    copy the data, and return the newly allocated memory which contains a copy
    of data. Because the data could need aligned, we will use aml_pool_alloc
    instead of aml_pool_ualloc */
  char *dest = (char *)aml_pool_alloc(h, len);
  if (len)
    memcpy(dest, data, len);
  return dest;
}

static inline char *aml_pool_strdupf(aml_pool_t *pool, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *r = aml_pool_strdupvf(pool, fmt, args);
  va_end(args);
  return r;
}

char **_aml_pool_split(aml_pool_t *h, size_t *num_splits, char delim, char *s);

struct aml_pool_marker_s {
  aml_pool_node_t *prev;
  char *curp;
  size_t size;
  size_t used;
#ifdef _AML_DEBUG_
  size_t cur_size;
#endif
};

static inline void aml_pool_save(aml_pool_t *h, aml_pool_marker_t *m) {
  m->prev = h->current->prev;
  m->curp = h->curp;
  m->size = h->size;
  m->used = h->used;
#ifdef _AML_DEBUG_
  m->cur_size = h->cur_size;
#endif
}

static inline void aml_pool_restore(aml_pool_t *h, aml_pool_marker_t *m) {
  /* remove the extra blocks (the ones where prev != NULL) */
  aml_pool_node_t *prev = h->current->prev;
  while (prev != m->prev) {
    if(!h->pool) {
#ifdef _AML_USE_MALLOC_
      free(h->current);
#else
      aml_free(h->current);
#endif
    }
    h->current = prev;
    prev = prev->prev;
  }

  /* reset to marker */
  h->curp = m->curp;
  h->size = m->size;

#ifdef _AML_DEBUG_
  h->cur_size = m->cur_size;
#endif
  h->used = m->used;
}
