// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdlib.h>

struct aml_buffer_s {
#ifdef _AML_DEBUG_
  aml_allocator_dump_t dump;
  size_t initial_size;
  size_t max_length;
#endif
  char *data;
  size_t length;
  size_t size;
  aml_pool_t *pool;
};

static inline aml_buffer_t *aml_buffer_pool_init(aml_pool_t *pool,
                                               size_t initial_size) {
  aml_buffer_t *h = (aml_buffer_t *)aml_pool_zalloc(pool, sizeof(aml_buffer_t));
  h->data = (char *)aml_pool_alloc(pool, initial_size + 1);
  h->data[0] = 0;
  h->size = initial_size;
  h->pool = pool;
  return h;
}

static inline
void aml_buffer_destroy(aml_buffer_t *h) {
  if (!h->pool) {
    /* If data points inside the object, it's the inline sentinel and must
       not be freed. Otherwise it's a heap buffer we own. */
    uintptr_t pb = (uintptr_t)h;
    uintptr_t pe = pb + sizeof(*h);
    uintptr_t pd = (uintptr_t)h->data;
    if (!(pd >= pb && pd < pe)) {
      aml_free(h->data);
    }
    aml_free(h);
  }
}


static inline
char *aml_buffer_detach(aml_buffer_t *h, size_t *length_out) {
    if (!h) {
        if (length_out) *length_out = 0;
        return NULL;
    }

    char *ret = NULL;
    size_t len = h->length;

    if (h->pool) {
        /* Pool-backed: return pool memory (caller must NOT free). */
        ret = h->data;

        /* Reset handle to an empty/sentinel state so it can be reused. */
        h->data   = (char *)&h->size;  /* sentinel points inside the struct */
        h->length = 0;
        h->size   = 0;
        h->data[0] = '\0';
    } else {
        /* Heap-backed */
        /* Detect if we were still using the inline sentinel. */
        uintptr_t pb = (uintptr_t)h;
        uintptr_t pe = pb + sizeof(*h);
        uintptr_t pd = (uintptr_t)h->data;
        bool is_sentinel = (pd >= pb && pd < pe);

        if (is_sentinel) {
            /* No real heap buffer yet: allocate a minimal heap buffer the
               caller can safely free. Length is 0, so 1 byte is fine. */
            size_t alloc = (len > 0) ? len : 1;
            ret = (char *)aml_malloc(alloc);
            /* Nothing to copy (no user data was stored); just NUL-terminate. */
            if (alloc > 0) ret[0] = '\0';
        } else {
            /* Transfer ownership of the existing heap allocation. */
            ret = h->data;
        }

        /* Restore sentinel on the handle so future appends/grows have a
           valid 1-byte source for memcpy of the NUL terminator. */
        h->data   = (char *)&h->size;
        h->length = 0;
        h->size   = 0;
        h->data[0] = '\0';
    }

    if (length_out) *length_out = len;
    return ret;
}


static inline void aml_buffer_clear(aml_buffer_t *h) {
  h->length = 0;
  h->data[0] = 0;
}

/* clear the buffer, freeing buffer if too large */
static inline void aml_buffer_reset(aml_buffer_t *h, size_t max_size) {
    if (h->size > max_size) {
        if (!h->pool) {
            aml_free(h->data);
            h->data = (char *)aml_malloc(max_size + 1);
            h->size = max_size;
        } // do nothing if pool
    }
    h->length = 0;
    h->data[0] = 0;
}


static inline char *aml_buffer_data(aml_buffer_t *h) {
    h->data[h->length] = 0;
    return h->data;
}
static inline size_t aml_buffer_length(aml_buffer_t *h) { return h->length; }
static inline char *aml_buffer_end(aml_buffer_t *h) {
  return h->data + h->length;
}

static inline void _aml_buffer_grow(aml_buffer_t *h, size_t length) {
  size_t len = (length + 50) + (h->size >> 3);
  if(len > 100*1024*1024)
    printf("aml_buffer_t: %p(%p): growing to %zu\n", (void*)h, (void*)h->pool, (size_t)len);
  if (!h->pool) {
    char *data = (char *)aml_malloc(len + 1);
    memcpy(data, h->data, h->length + 1);
    if (h->size)
      aml_free(h->data);
    h->data = data;
  } else {
    char *data = (char *)aml_pool_alloc(h->pool, len + 1);
    if(h->length)
        memcpy(data, h->data, h->length + 1);
    h->data = data;
  }
  h->size = len;
}

static inline void *aml_buffer_shrink_by(aml_buffer_t *h, size_t length) {
  if (h->length > length)
    h->length -= length;
  else
    h->length = 0;
  h->data[h->length] = 0;
  return h->data;
}

static inline void *aml_buffer_resize(aml_buffer_t *h, size_t length) {
  if (length > h->size)
    _aml_buffer_grow(h, length);
  h->length = length;
  h->data[h->length] = 0;
#ifdef _AML_DEBUG_
  if (length > h->max_length)
    h->max_length = length;
#endif
  return h->data;
}

static inline void *aml_buffer_append_alloc(aml_buffer_t *h, size_t length) {
  size_t m = h->length & 7;
  if (m > 0) {
    m = 8 - m;
    if (m + h->length > h->size)
      _aml_buffer_grow(h, m + h->length);
    h->length += m;
    h->data[h->length] = 0;
  }

  if (length + h->length > h->size)
    _aml_buffer_grow(h, length + h->length);
  char *r = h->data + h->length;
  h->length += length;
  r[length] = 0;
#ifdef _AML_DEBUG_
  if (h->length > h->max_length)
    h->max_length = h->length;
#endif
  return r;
}

static inline void *aml_buffer_append_ualloc(aml_buffer_t *h, size_t length) {
  if (length + h->length > h->size)
    _aml_buffer_grow(h, length + h->length);
  char *r = h->data + h->length;
  h->length += length;
  r[length] = 0;
#ifdef _AML_DEBUG_
  if (h->length > h->max_length)
    h->max_length = h->length;
#endif
  return r;
}

void _aml_buffer_append(aml_buffer_t *h, const void *data, size_t length);

static inline void aml_buffer_append(aml_buffer_t *h, const void *data,
                                    size_t length) {
  _aml_buffer_append(h, data, length);
}

static inline void aml_buffer_appends(aml_buffer_t *h, const char *s) {
  _aml_buffer_append(h, s, strlen(s));
}

static inline void aml_buffer_appendsz(aml_buffer_t *h, const char *s) {
  size_t length = strlen(s)+1;
  _aml_buffer_append(h, s, length);
}

static inline void aml_buffer_appendc(aml_buffer_t *h, char ch) {
  if (h->length + 1 > h->size)
    _aml_buffer_grow(h, h->length + 1);

  char *d = h->data + h->length;
  *d++ = ch;
  *d = 0;
  h->length++;
#ifdef _AML_DEBUG_
  if (h->length > h->max_length)
    h->max_length = h->length;
#endif
}

static inline void aml_buffer_appendn(aml_buffer_t *h, char ch, ssize_t n) {
  if (n <= 0)
    return;

  if (h->length + n > h->size)
    _aml_buffer_grow(h, h->length + n);

  char *d = h->data + h->length;
  memset(d, ch, n);
  d += n;
  *d = 0;
  h->length += n;
#ifdef _AML_DEBUG_
  if (h->length > h->max_length)
    h->max_length = h->length;
#endif
}

static inline void _aml_buffer_alloc(aml_buffer_t *h, size_t length) {
  size_t len = (length + 50) + (h->size >> 3);
  if (!h->pool) {
    if (h->size)
      aml_free(h->data);
    h->data = (char *)aml_malloc(len + 1);
  } else
    h->data = (char *)aml_pool_alloc(h->pool, len + 1);
  h->size = len;
}

static inline void *aml_buffer_alloc(aml_buffer_t *h, size_t length) {
  if (length > h->size)
    _aml_buffer_alloc(h, length);
  h->length = length;
#ifdef _AML_DEBUG_
  if (length > h->max_length)
    h->max_length = length;
#endif
  h->data[h->length] = 0;
  return h->data;
}

static inline void _aml_buffer_set(aml_buffer_t *h, const void *data,
                                  size_t length) {
  if (length > h->size)
    _aml_buffer_alloc(h, length);
  memcpy(h->data, data, length);
  h->length = length;
#ifdef _AML_DEBUG_
  if (length > h->max_length)
    h->max_length = length;
#endif
  h->data[length] = 0;
}

static inline void aml_buffer_set(aml_buffer_t *h, const void *data,
                                 size_t length) {
  _aml_buffer_set(h, data, length);
}

static inline void aml_buffer_sets(aml_buffer_t *h, const char *s) {
  _aml_buffer_set(h, s, strlen(s));
}

static inline void aml_buffer_setc(aml_buffer_t *h, char c) {
  _aml_buffer_set(h, &c, 1);
}

static inline void aml_buffer_setn(aml_buffer_t *h, char ch, ssize_t n) {
  h->length = 0;
  aml_buffer_appendn(h, ch, n);
}

static inline void aml_buffer_setvf(aml_buffer_t *h, const char *fmt,
                                   va_list args) {
  h->length = 0;
  aml_buffer_appendvf(h, fmt, args);
}

static inline void aml_buffer_setf(aml_buffer_t *h, const char *fmt, ...) {
  h->length = 0;
  va_list args;
  va_start(args, fmt);
  aml_buffer_appendvf(h, fmt, args);
  va_end(args);
}

static inline void aml_buffer_appendf(aml_buffer_t *h, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  aml_buffer_appendvf(h, fmt, args);
  va_end(args);
}
