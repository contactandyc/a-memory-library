// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "a-memory-library/aml_pool.h"

int main(void) {
    /* Initialize a memory pool with an initial size of 1024 bytes */
    aml_pool_t *pool = aml_pool_init(1024);
    if (!pool) {
        fprintf(stderr, "Failed to initialize memory pool\n");
        return 1;
    }

    /* Define the input data that we want to encode in Base64 */
    const char *input = "Hello, base64!";
    size_t input_len = strlen(input);
    unsigned char *input_data = (unsigned char *)input;

    /* Encode the input data into a Base64 string using the pool */
    char *encoded = aml_pool_base64_encode(pool, input_data, input_len);
    if (!encoded) {
        fprintf(stderr, "Base64 encoding failed\n");
        aml_pool_destroy(pool);
        return 1;
    }
    printf("Original: %s\n", input);
    printf("Encoded : %s\n", encoded);

    /* Decode the Base64 string back to its original binary representation */
    size_t decoded_len = 0;
    unsigned char *decoded = aml_pool_base64_decode(pool, &decoded_len, encoded);
    if (!decoded) {
        fprintf(stderr, "Base64 decoding failed\n");
        aml_pool_destroy(pool);
        return 1;
    }
    
    /* Null-terminate the decoded string for safe printing */
    decoded[decoded_len] = '\0';
    printf("Decoded : %s\n", decoded);

    /* Clean up the memory pool and exit */
    aml_pool_destroy(pool);
    return 0;
}
