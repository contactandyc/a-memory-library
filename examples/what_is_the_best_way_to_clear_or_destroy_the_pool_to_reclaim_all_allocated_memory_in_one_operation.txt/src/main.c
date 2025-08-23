// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "a-memory-library/aml_pool.h"

int main(void) {
    // Initialize the pool with an initial size (e.g., 1024 bytes).
    aml_pool_t *pool = aml_pool_init(1024);
    if (!pool) {
        fprintf(stderr, "Failed to initialize memory pool.\n");
        return 1;
    }

    // Allocate memory from the pool by duplicating a sample string.
    const char *original = "Memory pool demonstration";
    char *dup_str = aml_pool_strdup(pool, original);
    printf("Allocated string from pool: %s\n", dup_str);

    // Display pool usage before clearing/destroying.
    size_t poolSize = aml_pool_size(pool);
    printf("Pool size before clearing/destroying: %lu bytes\n", (unsigned long) poolSize);

    /*
     * The aml_pool API does not require individual frees for each allocation.
     * Instead, there are two primary methods to reclaim memory:
     *
     * 1. aml_pool_clear(pool): This resets the allocation pointer and frees any extra blocks
     *    that exceeded the initial allocation. The memory of previous allocations may still be
     *    valid, but should not be relied upon.
     *
     * 2. aml_pool_destroy(pool): This completely destroys the pool and frees all memory associated
     *    with it in one operation.
     *
     * The best way to reclaim all allocated memory in one operation is to call aml_pool_destroy().
     */

    // First, clear the pool to reset allocations
    aml_pool_clear(pool);
    printf("Pool cleared. Memory allocations within pool have been reset for reuse.\n");

    // Optionally, perform new allocations after clearing the pool
    char *new_str = aml_pool_strdup(pool, "New allocation after clear");
    printf("New string from pool after clear: %s\n", new_str);

    // Now, when the pool is no longer needed, destroy it to reclaim all allocated memory
    aml_pool_destroy(pool);
    printf("Pool destroyed. All allocated memory has been reclaimed in one operation.\n");

    return 0;
}
