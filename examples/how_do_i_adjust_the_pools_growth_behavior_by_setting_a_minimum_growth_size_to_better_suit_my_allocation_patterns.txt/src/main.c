// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include "a-memory-library/aml_pool.h"

int main(void) {
    /* Initialize the memory pool with an initial size of 1024 bytes */
    size_t initial_size = 1024;
    aml_pool_t *pool = aml_pool_init(initial_size);
    printf("Pool initialized with %zu bytes.\n", initial_size);
    printf("Initial pool used: %zu bytes.\n", aml_pool_used(pool));

    /* Adjust the pool's growth behavior by setting a new minimum growth size.
       This will cause the pool to allocate new blocks that are at least 2048 bytes
       when a new block is needed (e.g. if an allocation exceeds current capacity). */
    size_t new_growth = 2048;
    aml_pool_set_minimum_growth_size(pool, new_growth);
    printf("Pool's minimum growth size set to %zu bytes.\n", new_growth);

    /* Force an allocation that exceeds the remaining space in the current block
       to trigger the growth of the pool. This allocation size is chosen to be
       larger than what might be remaining so that a new block is allocated.
    */
    size_t alloc_size = 1500;
    char *block = (char *)aml_pool_alloc(pool, alloc_size);
    if (block) {
        sprintf(block, "This allocation of %zu bytes forced a growth of the pool!", alloc_size);
        printf("%s\n", block);
    } else {
        printf("Allocation failed.\n");
    }

    printf("Pool used memory after allocation: %zu bytes.\n", aml_pool_used(pool));

    /* Clear the pool to reset its state (free extra blocks and reset pointers) */
    aml_pool_clear(pool);
    printf("Pool cleared. Used memory: %zu bytes.\n", aml_pool_used(pool));

    /* Destroy the pool to free all memory associated with it */
    aml_pool_destroy(pool);
    printf("Pool destroyed.\n");

    return 0;
}
