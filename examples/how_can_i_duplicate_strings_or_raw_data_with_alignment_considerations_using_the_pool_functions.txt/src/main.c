// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdint.h>
#include "a-memory-library/aml_pool.h"

int main(void) {
    /* Step 1: Initialize the pool with an initial size of 1024 bytes */
    aml_pool_t *pool = aml_pool_init(1024);
    if (!pool) {
        fprintf(stderr, "Failed to initialize pool.\n");
        return 1;
    }

    /* Step 2: Duplicate a string using aml_pool_strdup */
    const char *original_str = "Hello, world!";
    char *duplicated_str = aml_pool_strdup(pool, original_str);
    printf("Original string: %s\n", original_str);
    printf("Duplicated string: %s\n", duplicated_str);

    /* Step 3: Duplicate raw data with alignment considerations using aml_pool_dup */
    const unsigned char raw_data[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    size_t data_length = sizeof(raw_data);
    unsigned char *duplicated_data = (unsigned char *)aml_pool_dup(pool, raw_data, data_length);
    printf("Duplicated raw data: ");
    for (size_t i = 0; i < data_length; i++) {
        printf("%02X ", duplicated_data[i]);
    }
    printf("\n");

    /* Step 4: Allocate aligned memory using aml_pool_aalloc. For example, allocate 32 bytes
       aligned to a 16 byte boundary (useful for SIMD or similar requirements) */
    void *aligned_mem = aml_pool_aalloc(pool, 16, 32);
    printf("Aligned memory address: %p\n", aligned_mem);
    if (((uintptr_t)aligned_mem % 16) == 0) {
        printf("Memory is properly aligned to 16 bytes.\n");
    } else {
        printf("Alignment error!\n");
    }

    /* Step 5: Cleanup the pool to reclaim memory */
    aml_pool_destroy(pool);
    return 0;
}
