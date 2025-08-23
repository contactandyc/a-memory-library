// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

/* main.c - Demonstrates creating a nested pool from an existing pool to manage temporary or scoped allocations. */

#include <stdio.h>
#include "a-memory-library/aml_pool.h"

int main(void) {
    /* Step 1: Initialize the main pool */
    size_t main_pool_size = 1024;
    aml_pool_t *mainPool = aml_pool_init(main_pool_size);
    if (!mainPool) {
        fprintf(stderr, "Failed to initialize main pool\n");
        return 1;
    }
    printf("Main pool created with size %lu\n", main_pool_size);

    /* Step 2: Create a nested pool from the main pool for temporary/scoped allocations */
    size_t nested_pool_size = 256;
    aml_pool_t *nestedPool = aml_pool_pool_init(mainPool, nested_pool_size);
    printf("Nested pool created with size %lu from main pool\n", nested_pool_size);

    /* Step 3: Perform allocations on the nested pool */
    char *tempString = aml_pool_strdup(nestedPool, "This is a temporary string from the nested pool");
    printf("Allocated in nested pool: %s\n", tempString);

    /* You can also use formatted allocations with the nested pool */
    char *tempFormatted = aml_pool_strdupf(nestedPool, "Number: %d, String: %s", 42, "Hello");
    printf("Formatted allocation in nested pool: %s\n", tempFormatted);

    /* Step 4: Once the temporary allocations are no longer required, clear the nested pool
       to reclaim the memory allocated within its scope. */
    aml_pool_clear(nestedPool);
    printf("Nested pool cleared\n");

    /* Step 5: Use the main pool for more permanent allocations if needed */
    char *permanentString = aml_pool_strdup(mainPool, "This is allocated in the main pool");
    printf("Permanent allocation in main pool: %s\n", permanentString);

    /* Step 6: Destroy the main pool, which will free all memory associated with it */
    aml_pool_destroy(mainPool);
    printf("Main pool destroyed\n");

    return 0;
}
