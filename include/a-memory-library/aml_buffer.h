// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#ifndef _aml_buffer_H
#define _aml_buffer_H

/*
  The aml_buffer object is much like the C++ string class in that it supports
  a number of easy to use string methods and a buffer acts like a string in
  that a buffer grows as needed to fit the contents of the string.  The object
  is more than a string object in as much as it also equally allows for
  binary data to be appended to or a combination of binary data and strings.
  At it's core, it is simply a buffer that will auto-resize as needed.  The
  pool object is different in that it is used for lots of smaller allocations.
  The buffer generally is used to track "one thing".  That one thing might
  consist of many parts, but they are generally all tracked together in a
  contiguous space.
*/

#include "a-memory-library/aml_alloc.h"
#include "a-memory-library/aml_pool.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

struct aml_buffer_s;
typedef struct aml_buffer_s aml_buffer_t;

/* aml_buffer_init creates a buffer with an initial size of size.  The buffer
   will grow as needed, but if you know the size that is generally needed,
   it may be more efficient to initialize it to that size.

   aml_buffer_t *aml_buffer_init(size_t size);
*/
#ifdef _AML_DEBUG_
#define aml_buffer_init(size)                                                   \
  _aml_buffer_init(size, aml_file_line_func("aml_buffer"));
aml_buffer_t *_aml_buffer_init(size_t size, const char *caller);
#else
#define aml_buffer_init(size) _aml_buffer_init(size)
aml_buffer_t *_aml_buffer_init(size_t size);
#endif

/* like above, except allocated with a pool (no need to destroy) */
static inline aml_buffer_t *aml_buffer_pool_init(aml_pool_t *pool,
                                               size_t initial_size);

/* destroy the buffer */
static inline
void aml_buffer_destroy(aml_buffer_t *h);

/* detach the buffer from the aml_buffer_t object.  The caller now owns the
   buffer and should free it when done.  The length of the buffer is returned
   in length_out.  If the buffer was allocated by a pool, it is an error to
   free the memory.*/
static inline
char *aml_buffer_detach(aml_buffer_t *h, size_t *length_out);

/* clear the buffer */
static inline void aml_buffer_clear(aml_buffer_t *h);

/* clear the buffer, freeing buffer if too large */
static inline void aml_buffer_reset(aml_buffer_t *h, size_t max_size);

/* resize the buffer and return a pointer to the beginning of the buffer.  This
   will retain the original data in the buffer for up to length bytes. */
static inline void *aml_buffer_resize(aml_buffer_t *h, size_t length);

/* shrink the buffer by length bytes, if the buffer is not length bytes, buffer
   will be cleared. */
static inline void *aml_buffer_shrink_by(aml_buffer_t *h, size_t length);

/* get the contents of the buffer */
static inline char *aml_buffer_data(aml_buffer_t *h);

/* get the length of the buffer */
static inline size_t aml_buffer_length(aml_buffer_t *h);

/* get the end of contents of the buffer */
static inline char *aml_buffer_end(aml_buffer_t *h);

/* Functions to set the contents into a buffer (vs append). */
/* set bytes to the current buffer */
static inline void aml_buffer_set(aml_buffer_t *h, const void *data,
                                 size_t length);

/* set a string to the current buffer */
static inline void aml_buffer_sets(aml_buffer_t *h, const char *s);

/* set a character to the current buffer */
static inline void aml_buffer_setc(aml_buffer_t *h, char ch);

/* set a character n times to the current buffer */
static inline void aml_buffer_setn(aml_buffer_t *h, char ch, ssize_t n);

/* set bytes in current buffer using va_args and formatted string */
static inline void aml_buffer_setvf(aml_buffer_t *h, const char *fmt,
                                   va_list args);

/* set bytes in current buffer using a formatted string -similar to printf */
static inline void aml_buffer_setf(aml_buffer_t *h, const char *fmt, ...);

/* Functions to append contents into a buffer (vs set). */
/* append bytes to the current buffer */
static inline void aml_buffer_append(aml_buffer_t *h, const void *data,
                                     size_t length);

/* append a string to the current buffer */
static inline void aml_buffer_appends(aml_buffer_t *h, const char *s);

/* append a string to the current buffer with zero terminator */
static inline void aml_buffer_appendsz(aml_buffer_t *h, const char *s);

/* append a character to the current buffer */
static inline void aml_buffer_appendc(aml_buffer_t *h, char ch);

/* append a character n times to the current buffer */
static inline void aml_buffer_appendn(aml_buffer_t *h, char ch, ssize_t n);

/* append bytes in current buffer using va_args and formatted string */
void aml_buffer_appendvf(aml_buffer_t *h, const char *fmt, va_list args);

/* append bytes in current buffer using a formatted string -similar to printf */
static inline void aml_buffer_appendf(aml_buffer_t *h, const char *fmt, ...);

/* grow the buffer by length bytes and return pointer to the new memory.  This
   will retain the original data in the buffer for up to length bytes. */
static inline void *aml_buffer_append_alloc(aml_buffer_t *h, size_t length);

/* same as append_alloc except memory is not necessarily aligned */
static inline void *aml_buffer_append_ualloc(aml_buffer_t *h, size_t length);

/* resize the buffer and return a pointer to the beginning of the buffer.  This
   will NOT retain the original data in the buffer for up to length bytes. */
static inline void *aml_buffer_alloc(aml_buffer_t *h, size_t length);

#include "a-memory-library/impl/aml_buffer.h"

#ifdef __cplusplus
}
#endif

#endif
