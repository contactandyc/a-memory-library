// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "a-memory-library/aml_pool.h"

int main(void) {
    /* Initialize an AML pool with 1024 bytes */
    aml_pool_t *pool = aml_pool_init(1024);
    if (!pool) {
        fprintf(stderr, "Failed to initialize memory pool.\n");
        return EXIT_FAILURE;
    }

    /* Example 1: Basic split using aml_pool_split */
    const char *str1 = "one,two,three";
    size_t count1 = 0;
    char **tokens1 = aml_pool_split(pool, &count1, ',', str1);
    printf("\nBasic Split (aml_pool_split) on '%s':\n", str1);
    for (size_t i = 0; tokens1[i] != NULL; i++) {
        printf("Token %zu: '%s'\n", i, tokens1[i]);
    }

    /* Example 2: Split ignoring empty tokens using aml_pool_split2 */
    const char *str2 = "a,,b,,,c";
    size_t count2 = 0;
    char **tokens2 = aml_pool_split2(pool, &count2, ',', str2);
    printf("\nSplit Ignoring Empty Tokens (aml_pool_split2) on '%s':\n", str2);
    for (size_t i = 0; tokens2[i] != NULL; i++) {
        printf("Token %zu: '%s'\n", i, tokens2[i]);
    }

    /* Example 3: Split with escape support using aml_pool_split_with_escape */
    // In this example the sequence "\," (escaped comma) is treated as a literal comma
    const char *str3 = "one,two\,with\,commas,three";
    size_t count3 = 0;
    char **tokens3 = aml_pool_split_with_escape(pool, &count3, ',', '\\', str3);
    printf("\nSplit with Escape (aml_pool_split_with_escape) on '%s':\n", str3);
    for (size_t i = 0; tokens3[i] != NULL; i++) {
        printf("Token %zu: '%s'\n", i, tokens3[i]);
    }

    /* Example 4: Split with escape & ignoring empty tokens using aml_pool_split_with_escape2 */
    const char *str4 = "first,,second,third\,part,,,fourth";
    size_t count4 = 0;
    char **tokens4 = aml_pool_split_with_escape2(pool, &count4, ',', '\\', str4);
    printf("\nSplit with Escape and Ignoring Empty Tokens (aml_pool_split_with_escape2) on '%s':\n", str4);
    for (size_t i = 0; tokens4[i] != NULL; i++) {
        printf("Token %zu: '%s'\n", i, tokens4[i]);
    }

    /* Summary of available options for splitting strings */
    printf("\nAvailable string splitting functions in AML Pool library:\n");
    printf("- aml_pool_split: Basic splitting using a delimiter.\n");
    printf("- aml_pool_split2: Splitting that ignores empty tokens.\n");
    printf("- aml_pool_split_with_escape: Splitting using an escape character to ignore escaped delimiters.\n");
    printf("- aml_pool_split_with_escape2: Splitting with escape support that also ignores empty tokens.\n");

    /* Clean up the pool */
    aml_pool_destroy(pool);
    return EXIT_SUCCESS;
}
