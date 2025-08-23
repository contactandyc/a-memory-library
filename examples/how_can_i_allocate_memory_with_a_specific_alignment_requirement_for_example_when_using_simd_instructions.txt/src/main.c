// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "a-memory-library/aml_pool.h"

int main(void) {
    // Initialize a memory pool with an initial size of 1024 bytes
    aml_pool_t *pool = aml_pool_init(1024);
    if (!pool) {
        fprintf(stderr, "Failed to initialize memory pool.\n");
        return 1;
    }

    // SIMD instructions often require memory to be aligned to a specific boundary.
    // Here we require a 64-byte aligned allocation and request 256 bytes.
    size_t alignment = 64;  // Example alignment requirement for SIMD instructions
    size_t allocation_size = 256; 

    // Allocate memory from the pool with the specific alignment using aml_pool_aalloc
    void *aligned_memory = aml_pool_aalloc(pool, alignment, allocation_size);
    if (!aligned_memory) {
        fprintf(stderr, "Aligned allocation failed.\n");
        aml_pool_destroy(pool);
        return 1;
    }

    // Verify that the memory is aligned by checking the address modulo the alignment
    if (((uintptr_t)aligned_memory % alignment) == 0) {
        printf("Memory allocated at address %p is aligned to %zu bytes.\n", aligned_memory, alignment);
    } else {
        printf("Memory allocated at address %p is NOT aligned properly.\n", aligned_memory);
    }

    // Use the allocated memory (for demonstration, we zero initialize it)
    memset(aligned_memory, 0, allocation_size);

    // Once done, clear and destroy the pool to free up the allocated memory
    aml_pool_clear(pool);
    aml_pool_destroy(pool);

    return 0;
}
