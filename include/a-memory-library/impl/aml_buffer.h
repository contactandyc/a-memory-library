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

static inline void aml_buffer_clear(aml_buffer_t *h) {
  h->length = 0;
  h->data[0] = 0;
}

static inline char *aml_buffer_data(aml_buffer_t *h) { return h->data; }
static inline size_t aml_buffer_length(aml_buffer_t *h) { return h->length; }
static inline char *aml_buffer_end(aml_buffer_t *h) {
  return h->data + h->length;
}

static inline void _aml_buffer_grow(aml_buffer_t *h, size_t length) {
  size_t len = (length + 50) + (h->size >> 3);
  if (!h->pool) {
    char *data = (char *)aml_malloc(len + 1);
    memcpy(data, h->data, h->length + 1);
    if (h->size)
      aml_free(h->data);
    h->data = data;
  } else
    h->data = (char *)aml_pool_dup(h->pool, h->data, len + 1);
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
