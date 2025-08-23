// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "a-memory-library/aml_pool.h"

// Define a sample data structure
typedef struct {
    int value;
    char *name;
} MyData;

int main(void) {
    // Initialize the memory pool with an initial size of 1024 bytes
    aml_pool_t *pool = aml_pool_init(1024);
    if (!pool) {
        fprintf(stderr, "Failed to initialize memory pool\n");
        exit(EXIT_FAILURE);
    }
    
    // Allocate memory for our data structure from the pool
    MyData *data = (MyData *)aml_pool_alloc(pool, sizeof(MyData));
    if (!data) {
        fprintf(stderr, "Failed to allocate memory for MyData\n");
        aml_pool_destroy(pool);
        exit(EXIT_FAILURE);
    }
    
    // Set fields of the structure
    data->value = 42;
    // Duplicate a string into the pool for our data structure
    data->name = aml_pool_strdup(pool, "MemoryPoolExample");
    
    // Print the contents of the data structure
    printf("Data value: %d\n", data->value);
    printf("Data name: %s\n", data->name);
    
    // Optionally, clear the pool to reuse memory without full destroy
    // aml_pool_clear(pool);
    
    // Finally, destroy the pool to reclaim all allocated memory
    aml_pool_destroy(pool);
    
    return 0;
}
