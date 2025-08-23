// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include "a-memory-library/aml_pool.h"

int main(void) {
    // Initialize a memory pool with 1024 bytes
    aml_pool_t *pool = aml_pool_init(1024);
    
    // Allocate a persistent string that will remain valid
    char *persistent = aml_pool_strdup(pool, "Persistent allocation");
    printf("Before save: %s\n", persistent);
    
    // Save the current state of the pool
    aml_pool_marker_t marker;
    aml_pool_save(pool, &marker);
    
    // Allocate temporary memory; these allocations are intended to be undone
    char *temporary = aml_pool_strdup(pool, "Temporary allocation");
    printf("Temporary allocation: %s\n", temporary);
    
    char *anotherTemp = aml_pool_strdup(pool, "Another temporary allocation");
    printf("Another temporary allocation: %s\n", anotherTemp);
    
    // Restore the pool to the state saved previously, rolling back temporary allocations
    aml_pool_restore(pool, &marker);
    
    // The persistent allocation is still valid, while the temporary ones are now invalid
    printf("After restore, persistent allocation: %s\n", persistent);
    
    // Allocate new memory after restore to demonstrate continued usage
    char *postRestore = aml_pool_strdup(pool, "Post-restore allocation");
    printf("New allocation after restore: %s\n", postRestore);
    
    // Clean up the pool and free all memory
    aml_pool_destroy(pool);
    
    return 0;
}
