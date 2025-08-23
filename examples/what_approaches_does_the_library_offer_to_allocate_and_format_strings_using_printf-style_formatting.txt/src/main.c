// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdarg.h>
#include "a-memory-library/aml_pool.h"

/*
 * This program demonstrates the different approaches provided by the AML pool
 * library to allocate and format strings using printf-style formatting. The library
 * offers two main functions for this purpose:
 *
 * 1. aml_pool_strdupf: which takes a format string and variadic arguments to produce
 *    a formatted string allocated from the memory pool.
 *
 * 2. aml_pool_strdupvf: which accepts a va_list for formatted allocation. A helper
 *    function (demo_va) is provided to wrap this functionality.
 *
 * Both approaches allow for efficient memory management by allocating all memory
 * from a memory pool, which is cleared or destroyed as needed, eliminating the need
 * for individual calls to free.
 */

/* Helper function that demonstrates the use of aml_pool_strdupvf with a va_list */
char* demo_va(aml_pool_t *pool, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *result = aml_pool_strdupvf(pool, format, args);
    va_end(args);
    return result;
}

int main(void) {
    /* Initialize the memory pool with an initial size of 1024 bytes */
    aml_pool_t *pool = aml_pool_init(1024);
    if (!pool) {
        fprintf(stderr, "Failed to initialize memory pool\n");
        return 1;
    }

    /* Approach 1: Use aml_pool_strdupf to format a string and allocate it from the pool */
    char *str1 = aml_pool_strdupf(pool, "Hello, %s! The value is %d.", "World", 42);
    printf("Using aml_pool_strdupf: %s\n", str1);

    /* Approach 2: Use the helper function demo_va which internally calls aml_pool_strdupvf */
    char *str2 = demo_va(pool, "Formatted using va_list: Pi is approximately %.2f", 3.14159);
    printf("Using aml_pool_strdupvf: %s\n", str2);

    /* Clean up the memory pool. All allocated memory is reclaimed by destroying the pool. */
    aml_pool_destroy(pool);

    return 0;
}
