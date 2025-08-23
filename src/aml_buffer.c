// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include "a-memory-library/aml_buffer.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _AML_DEBUG_
static void dump_buffer(FILE *out, const char *caller, void *p, size_t length) {
  aml_buffer_t *bh = (aml_buffer_t *)p;
  fprintf(out, "%s size: %lu, max_length: %lu, initial_size: %lu ", caller,
          bh->size, bh->max_length, bh->initial_size);
}

aml_buffer_t *_aml_buffer_init(size_t initial_size, const char *caller) {
  aml_buffer_t *h = (aml_buffer_t *)_aml_malloc_d(
      caller, sizeof(aml_buffer_t), true);
  h->dump.dump = dump_buffer;
  h->initial_size = initial_size;
  h->max_length = 0;
#else
aml_buffer_t *_aml_buffer_init(size_t initial_size) {
  aml_buffer_t *h = (aml_buffer_t *)aml_malloc(sizeof(aml_buffer_t));
#endif
  h->data = initial_size ? (char *)aml_malloc(initial_size + 1)
                         : (char *)(&(h->size));
  h->data[0] = 0;
  h->length = 0;
  h->size = initial_size;
  h->pool = NULL;
  return h;
}

void _aml_buffer_append(aml_buffer_t *h, const void *data, size_t length) {
  if (h->length + length > h->size)
    _aml_buffer_grow(h, h->length + length);

  memcpy(h->data + h->length, data, length);
  h->length += length;
  h->data[h->length] = 0;
#ifdef _AML_DEBUG_
  if (length > h->max_length)
    h->max_length = length;
#endif
}

void aml_buffer_appendvf(aml_buffer_t *h, const char *fmt, va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);
  size_t leftover = h->size - h->length;
  char *r = h->data + h->length;
  int n = vsnprintf(r, leftover, fmt, args_copy);
  if (n < 0)
    abort();
  va_end(args_copy);
  if (n < leftover)
    h->length += n;
  else {
    _aml_buffer_grow(h, h->length + n);
    r = h->data + h->length;
    va_copy(args_copy, args);
    int n2 = vsnprintf(r, n + 1, fmt, args_copy);
    if (n != n2)
      abort(); // should never happen!
    va_end(args_copy);
    h->length += n;
  }
#ifdef _AML_DEBUG_
  if (h->length > h->max_length)
    h->max_length = h->length;
#endif
}
