// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#ifndef _aml_alloc_H
#define _aml_alloc_H

#define _AML_TO_STRING(x) #x
#define AML_TO_STRING(x) _AML_TO_STRING(x)
#define aml_file_line() __FILE__ ":" AML_TO_STRING(__LINE__)
#define aml_file_line_func(a) aml_file_line() " [" a "]"

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _AML_DEBUG_
#define aml_dump(out) _aml_dump(out)

#define aml_alloc_log(filename) _aml_alloc_log(filename)

#define aml_malloc(len) _aml_malloc_d(aml_file_line(), len, false)
#define aml_zalloc(len) _aml_calloc_d(aml_file_line(), len, false)
#define aml_calloc(num_items, size) _aml_calloc_d(aml_file_line(), num_items*size, false)
#define aml_realloc(p, len) _aml_realloc_d(aml_file_line(), p, len, false)
#define aml_strdup(p) _aml_strdup_d(aml_file_line(), p)
#define aml_strdupf(p, ...) _aml_strdupf_d(aml_file_line(), p, __VA_ARGS__)
#define aml_strdupvf(p, args) _aml_strdupvf_d(aml_file_line(), p, args)
#define aml_strdupa(p) _aml_strdupa_d(aml_file_line(), p)
#define aml_strdupan(p, n) _aml_strdupan_d(aml_file_line(), p, n)
#define aml_strdupa2(p) _aml_strdupa2_d(aml_file_line(), p)
#define aml_dup(p, len) _aml_dup_d(aml_file_line(), p, len)
#define aml_free(p) _aml_free_d(aml_file_line(), p)
#else
#define aml_dump(out) ;
#define aml_alloc_log(filename) ;
#define aml_malloc(len) malloc(len)
#define aml_zalloc(len) calloc(1, len)
#define aml_calloc(num_items, size) calloc(num_items, size)
#define aml_realloc(p, len) realloc(p, len)
#define aml_strdup(p) strdup(p)
#define aml_strdupf(p, ...) _aml_strdupf(p, __VA_ARGS__)
#define aml_strdupvf(p, args) _aml_strdupvf(p, args)
#define aml_strdupa(p) _aml_strdupa(p)
#define aml_strdupan(p, n) _aml_strdupan(p, n)
#define aml_strdupa2(p) _aml_strdupa2(p)
#define aml_dup(p, len) _aml_dup(p, len)
#define aml_free(p) free(p)
#endif

void _aml_dump(FILE *out);

void _aml_alloc_log(const char *filename);

typedef void (*aml_dump_details_cb)(FILE *out, const char *caller, void *p,
                                  size_t length);

typedef struct {
  aml_dump_details_cb dump;
} aml_allocator_dump_t;

char *_aml_strdupf_d(const char *caller, const char *p, ...);
char *_aml_strdupf(const char *p, ...);
char *_aml_strdupvf_d(const char *caller, const char *p, va_list args);
char *_aml_strdupvf(const char *p, va_list args);

char **_aml_strdupa_d(const char *caller, char **a);
char **_aml_strdupan_d(const char *caller, char **a, size_t n);
char **_aml_strdupa(char **a);
char **_aml_strdupan(char **a, size_t n);

char **_aml_strdupa2_d(const char *caller, char **a);
char **_aml_strdupa2(char **a);

void *_aml_malloc_d(const char *caller, size_t len, bool custom);

void *_aml_calloc_d(const char *caller, size_t len, bool custom);

void *_aml_realloc_d(const char *caller, void *p, size_t len, bool custom);

char *_aml_strdup_d(const char *caller, const char *p);

void _aml_free_d(const char *caller, void *p);

static inline void *_aml_dup_d(const char *caller,
                                  const void *p, size_t len) {
  void *r = _aml_malloc_d(caller, len, false);
  memcpy(r, p, len);
  return r;
}

static inline void *_aml_dup(const void *p, size_t len) {
  void *r = malloc(len);
  memcpy(r, p, len);
  return r;
}

static inline
void _aml_free(void *p) {
#ifdef _AML_DEBUG_
    _aml_free_d(aml_file_line(), p);
#else
    free(p);
#endif
}

#ifdef __cplusplus
}
#endif

#endif
