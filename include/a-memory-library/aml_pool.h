// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

/*
  The aml_pool provides an api similar to malloc, calloc, strdup, along with many
  other useful common allocation patterns with the exception of free.  The pool
  must be cleared (aml_pool_clear) or destroyed (aml_pool_destroy) to reclaim
  memory allocated.

  C doesn't have garbage collection.  Many C developers prefer to stay away from
  languages which have it as it can cause performance issues.  The pool provides
  a way for allocations to happen without them being tracked.  Collection is not
  automatic.  The pool must be cleared or destroyed for memory to be reclaimed.
  This affords the end user significant performance advantages in that each pool
  can be cleared independently.  Pools can be created per thread and at various
  scopes providing a mechanism to mostly (if not completely) eliminate memory
  fragmentation.  The pool object isn't thread safe.  In general, locks cause
  problems and if code can be designed to be thread safe without locking, it
  will perform better.  Many of the objects within the aml_ collection will use
  the pool for allocation for all of the reasons mentioned above.

  Clearing the pool generally consists of resetting a pointer.  Memory is only
  freed if the memory used by the pool exceeded the initial size assigned to it
  during initialization.  In this case, the extra blocks will be freed before
  the counter is reset.  It is normally best to set the initial size so that
  overflowing doesn't happen, except in rare circumstances.  The memory that was
  previously allocated prior to a clear will still possibly be valid, but
  shouldn't be relied upon.
*/

#ifndef _aml_pool_H
#define _aml_pool_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "a-memory-library/aml_alloc.h"

// #ifndef _AML_USE_MALLOC_
// #define _AML_USE_MALLOC_
// #endif

#ifdef __cplusplus
extern "C" {
#endif

struct aml_pool_s;
typedef struct aml_pool_s aml_pool_t;

/* aml_pool_init will create a working space of size bytes */
#ifdef _AML_DEBUG_
#define aml_pool_init(size) _aml_pool_init(size, aml_file_line_func("aml_pool"))
aml_pool_t *_aml_pool_init(size_t size, const char *caller);
#else
#define aml_pool_init(size) _aml_pool_init(size)
aml_pool_t *_aml_pool_init(size_t size);
#endif

/* aml_pool_pool_init creates a pool from another pool.  This can be useful for
   having a repeated clearing mechanism inside a larger pool.  Ideally, this
   pool should be sized right as the clear function can't free nodes. */
aml_pool_t *aml_pool_pool_init(aml_pool_t *pool, size_t initial_size);


/* aml_pool_clear will make all of the pool's memory reusable.  If the
  initial block was exceeded and additional blocks were added, those blocks
  will be freed. */
void aml_pool_clear(aml_pool_t *h);

/* aml_pool_destroy frees up all memory associated with the pool object */
void aml_pool_destroy(aml_pool_t *h);

struct aml_pool_marker_s;
typedef struct aml_pool_marker_s aml_pool_marker_t;

/* Use the aml_pool_marker_t to save the current pool state and then return to it later with restore.
   This is useful if allocation after the marker is no longer needed (like the stack paradigm) */
static inline void aml_pool_save(aml_pool_t *h, aml_pool_marker_t *cp);
static inline void aml_pool_restore(aml_pool_t *h, aml_pool_marker_t *cp);


/* aml_pool_set_minimum_growth_size alters the minimum size of growth blocks.
   This is particularly useful if you don't expect the pool's block size to be
   exceeded by much and you don't want the default which would be to use the
   original block size for the new block (effectively doubling memory usage). */
void aml_pool_set_minimum_growth_size(aml_pool_t *h, size_t size);

/* aml_pool_alloc allocates len uninitialized bytes which are aligned. */
static inline void *aml_pool_alloc(aml_pool_t *h, size_t len);

/* aml_pool_aalloc allocates len unitialized bytes which are aligned by 
   alignment.  Alignment must be a non-zero, power of two.  Primary use
   for this is for SIMD instructions where alignment would be 64. */
void *aml_pool_aalloc(aml_pool_t *pool, size_t alignment, size_t len);

/* aml_pool_min_max_alloc allocates at least min_len bytes and up to len bytes.
   If the */
static inline void *aml_pool_min_max_alloc(aml_pool_t *h, size_t *rlen,
                                          size_t min_len, size_t len);

/* aml_pool_alloc allocates len uninitialized bytes which are unaligned. */
static inline void *aml_pool_ualloc(aml_pool_t *h, size_t len);

/* aml_pool_alloc allocates len zero'd bytes which are aligned. */
static inline void *aml_pool_zalloc(aml_pool_t *h, size_t len);

/* aml_pool_alloc allocates len zero'd bytes which are aligned. */
static inline void *aml_pool_calloc(aml_pool_t *h, size_t num_items, size_t size);

/* aml_pool_strdup allocates a copy of the string p.  The memory will be
  unaligned.  If you need the memory to be aligned, consider using aml_pool_dup
  like char *s = aml_pool_dup(pool, p, strlen(p)+1); */
static inline char *aml_pool_strdup(aml_pool_t *h, const char *p);

/* aml_pool_strdupf allocates a copy of the formatted string p. */
static inline char *aml_pool_strdupf(aml_pool_t *h, const char *p, ...);

/* aml_pool_strdupvf allocates a copy of the formatted string p. This is
  particularly useful if you wish to extend another object which uses pool as
  its base.  */
char *aml_pool_strdupvf(aml_pool_t *h, const char *p, va_list args);

/* like aml_pool_strdup, limited to length (+1 for zero terminator) bytes */
static inline char *aml_pool_strndup(aml_pool_t *h, const char *p, size_t length);

/* aml_pool_dup allocates a copy of the data.  The memory will be aligned. */
static inline void *aml_pool_dup(aml_pool_t *h, const void *data, size_t len);

/* aml_pool_dup allocates a copy of the data.  The memory will be unaligned. */
static inline void *aml_pool_udup(aml_pool_t *h, const void *data, size_t len);

/* aml_pool_size returns the number of bytes that have been allocated from any
  of the alloc calls above.  */
size_t aml_pool_size(aml_pool_t *h);

/* aml_pool_used returns the number of bytes that have been allocated by the
  pool itself.  This will always be greater than aml_pool_size as there is
  overhead for the structures and this is independent of any allocating calls.
*/
size_t aml_pool_used(aml_pool_t *h);

/* split a string into N pieces using delimiter.  The array that is returned
   will always be valid with a NULL string at the end if p is NULL. num_splits
   can be NULL if the number of returning pieces is not desired. */
char **aml_pool_split(aml_pool_t *h, size_t *num_splits, char delim,
                     const char *p);

/* same as aml_split except allows formatting of input string. */
char **aml_pool_splitf(aml_pool_t *h, size_t *num_splits, char delim,
                      const char *p, ...);

/* same as aml_split except empty strings will not be included in the
   result. */
char **aml_pool_split2(aml_pool_t *h, size_t *num_splits, char delim,
                      const char *p);

/* same as aml_split2 except allows formatting of input string. */
char **aml_pool_split2f(aml_pool_t *h, size_t *num_splits, char delim,
                       const char *p, ...);

/* split a string into N pieces using delimiter.  The array that is returned
   will always be valid with a NULL string at the end if p is NULL. num_splits
   can be NULL if the number of returning pieces is not desired.  This version
   allows for an escape character to be used to escape the delimiter. */
char **aml_pool_split_with_escape(aml_pool_t *h, size_t *num_splits, char delim, char escape, const char *p);

/* same as aml_split_with_escape except allows formatting of input string. */
char **aml_pool_split_with_escapef(aml_pool_t *h, size_t *num_splits, char delim, char escape, const char *p, ...);

/* split a string into N pieces using delimiter.  The array that is returned
   will always be valid with a NULL string at the end if p is NULL. num_splits
   can be NULL if the number of returning pieces is not desired.  This version
   allows for an escape character to be used to escape the delimiter.  It also removes
   all empty strings. */
char **aml_pool_split_with_escape2(aml_pool_t *h, size_t *num_splits, char delim, char escape, const char *s);

/* same as aml_split_with_escape2 except allows formatting of input string. */
char **aml_pool_split_with_escape2f(aml_pool_t *h, size_t *num_splits, char delim, char escape,
                                    const char *p, ...);

/* duplicate all of the strings in arr AND the NULL terminated pointer array.  */
char **aml_pool_strdupa(aml_pool_t *pool, char **arr);

/* duplicate all of the strings in arr AND the NULL terminated pointer array.  */
char **aml_pool_strdupan(aml_pool_t *pool, char **arr, size_t num);

/* Duplicate the NULL terminated pointer array. */
char **aml_pool_strdupa2(aml_pool_t *pool, char **arr);

/* aml_pool_base64_encode encodes data into base64.  The result will be
   null terminated base64 string which represents data. */
char *aml_pool_base64_encode(aml_pool_t *pool, const unsigned char *data, size_t data_len);

/* aml_pool_base64_decode decodes a base64 string into binary data.  The result
   will be null terminated.  The length of the binary data will be returned in
   out_len. */
unsigned char *aml_pool_base64_decode(aml_pool_t *pool, size_t *out_len, const char *b64);

#include "a-memory-library/impl/aml_pool.h"

#ifdef __cplusplus
}
#endif

#endif
