// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

// test_aml_alloc.c
#include "the-macro-library/macro_test.h"
#include "a-memory-library/aml_alloc.h"

#include <string.h>
#include <stdarg.h>

#ifdef _AML_DEBUG_
#define ASSERT_ZERO_ALLOC_RETURNS_NULL(expr) MACRO_ASSERT_TRUE((expr) == NULL)
#else
#define ASSERT_ZERO_ALLOC_RETURNS_NULL(expr) \
    do { void *tmp = (expr); if (tmp) aml_free(tmp); } while (0)
#endif

static char *vdup(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *s = aml_strdupvf(fmt, args);
    va_end(args);
    return s;
}

MACRO_TEST(alloc_malloc_zalloc_calloc_free) {
    char *p = (char*)aml_malloc(37);
    MACRO_ASSERT_TRUE(p); memset(p, 0xBE, 37); aml_free(p);

    int *a = (int*)aml_zalloc(8*sizeof(int));
    for (int i=0;i<8;i++) MACRO_ASSERT_EQ_INT(a[i], 0);
    aml_free(a);

    int *b = (int*)aml_calloc(7, sizeof(int));
    for (int i=0;i<7;i++) MACRO_ASSERT_EQ_INT(b[i], 0);
    aml_free(b);
}

MACRO_TEST(alloc_strdup_family_and_dup) {
    char *s1 = aml_strdup("abc"); MACRO_ASSERT_STREQ(s1, "abc"); aml_free(s1);

    char *s2 = aml_strdupf("x=%d %s", 5, "ok");
    MACRO_ASSERT_STREQ(s2, "x=5 ok"); aml_free(s2);

    char *s3 = vdup("pi=%.2f", 3.14159);
    MACRO_ASSERT_STREQ(s3, "pi=3.14"); aml_free(s3);

    unsigned char in[5] = {1,2,3,4,5};
    unsigned char *copy = (unsigned char*)aml_dup(in, sizeof(in));
    MACRO_ASSERT_TRUE(memcmp(copy, in, sizeof(in)) == 0);
    aml_free(copy);
}

MACRO_TEST(alloc_realloc_grow_and_shrink_and_nullptr) {
    unsigned char *p = (unsigned char*)aml_realloc(NULL, 16);
    for (size_t i=0;i<16;i++) p[i]=(unsigned char)i;

    p = (unsigned char*)aml_realloc(p, 64);
    for (size_t i=0;i<16;i++) MACRO_ASSERT_EQ_INT(p[i], (int)i);

    p = (unsigned char*)aml_realloc(p, 8);
    for (size_t i=0;i<8;i++) MACRO_ASSERT_EQ_INT(p[i], (int)i);
    aml_free(p);
}

MACRO_TEST(alloc_strdupa_variants) {
    char *src[] = {"a", "bb", "ccc", NULL};

    char **a1 = aml_strdupa(src);
    MACRO_ASSERT_TRUE(a1 != src);
    MACRO_ASSERT_STREQ(a1[0],"a");
    MACRO_ASSERT_STREQ(a1[1],"bb");
    MACRO_ASSERT_STREQ(a1[2],"ccc");
    MACRO_ASSERT_TRUE(a1[3] == NULL);
    aml_free(a1);

    char **a2 = aml_strdupan(src, 2);
    MACRO_ASSERT_STREQ(a2[0],"a");
    MACRO_ASSERT_STREQ(a2[1],"bb");
    MACRO_ASSERT_TRUE(a2[2] == NULL);
    aml_free(a2);

    char **a3 = aml_strdupa2(src);
    MACRO_ASSERT_TRUE(a3 != src);
    MACRO_ASSERT_TRUE(a3[0] == src[0] && a3[1] == src[1] && a3[2] == src[2] && a3[3] == src[3]);
    aml_free(a3);
}

MACRO_TEST(alloc_dump_safe_to_call) {
    aml_dump(stdout);
}

MACRO_TEST(alloc_zero_length_allocations) {
    ASSERT_ZERO_ALLOC_RETURNS_NULL(aml_malloc(0));
    ASSERT_ZERO_ALLOC_RETURNS_NULL(aml_calloc(0, sizeof(int)));
    ASSERT_ZERO_ALLOC_RETURNS_NULL(aml_zalloc(0));
    ASSERT_ZERO_ALLOC_RETURNS_NULL(aml_realloc(NULL, 0));
}

MACRO_TEST(alloc_strdupa_variants_with_null_in_middle) {
    char *src[] = {"x", NULL, "y", NULL};

    char **a1 = aml_strdupan(src, 4);
    MACRO_ASSERT_STREQ(a1[0], "x");
    MACRO_ASSERT_TRUE(a1[1] == NULL);
    MACRO_ASSERT_STREQ(a1[2], "y");
    MACRO_ASSERT_TRUE(a1[3] == NULL);
    MACRO_ASSERT_TRUE(a1[4] == NULL);
    aml_free(a1);
}

MACRO_TEST(alloc_strdupa2_null_array) {
    char **res = aml_strdupa2(NULL);
    MACRO_ASSERT_TRUE(res == NULL);
}

MACRO_TEST(alloc_free_nullptr_is_noop) {
    aml_free(NULL);
}

MACRO_TEST(alloc_realloc_null_and_zero) {
    char *p = (char *)aml_realloc(NULL, 10);
    MACRO_ASSERT_TRUE(p);
    aml_free(p);

    p = (char *)aml_malloc(5);
    strcpy(p, "hi");
    ASSERT_ZERO_ALLOC_RETURNS_NULL(aml_realloc(p, 0));
}

MACRO_TEST(alloc_strdup_empty_and_onechar) {
    char *empty = aml_strdup("");
    MACRO_ASSERT_STREQ(empty, "");
    aml_free(empty);

    char *single = aml_strdup("x");
    MACRO_ASSERT_STREQ(single, "x");
    aml_free(single);
}

MACRO_TEST(alloc_strdupf_empty_format) {
    char *res = aml_strdupf("%s", "");
    MACRO_ASSERT_STREQ(res, "");
    aml_free(res);
}

MACRO_TEST(alloc_strdupvf_variants) {
    char *s = vdup("num=%d", 42);
    MACRO_ASSERT_STREQ(s, "num=42");
    aml_free(s);
}

MACRO_TEST(alloc_dup_binary_data) {
    unsigned char data[4] = {0xFF, 0x00, 0xAA, 0x55};
    unsigned char *copy = (unsigned char*)aml_dup(data, sizeof(data));
    MACRO_ASSERT_TRUE(memcmp(data, copy, sizeof(data)) == 0);
    aml_free(copy);
}

MACRO_TEST(alloc_strdupa_empty_array) {
    char *arr[] = {NULL};
    char **dup = aml_strdupa(arr);
    MACRO_ASSERT_TRUE(dup[0] == NULL);
    aml_free(dup);
}

MACRO_TEST(alloc_strdupan_handles_nulls_in_middle) {
    char *arr[] = {"a", NULL, "b", NULL};
    char **dup = aml_strdupan(arr, 4);
    MACRO_ASSERT_STREQ(dup[0], "a");
    MACRO_ASSERT_TRUE(dup[1] == NULL);
    MACRO_ASSERT_STREQ(dup[2], "b");
    MACRO_ASSERT_TRUE(dup[3] == NULL);
    MACRO_ASSERT_TRUE(dup[4] == NULL);
    aml_free(dup);
}

MACRO_TEST(alloc_strdupa2_empty_array) {
    char *arr[] = {NULL};
    char **dup = aml_strdupa2(arr);
    MACRO_ASSERT_TRUE(dup[0] == NULL);
    aml_free(dup);
}

MACRO_TEST(alloc_strdupa2_null_input) {
    char **dup = aml_strdupa2(NULL);
    MACRO_ASSERT_TRUE(dup == NULL);
}

MACRO_TEST(alloc_dup_and_free) {
    unsigned char buf[3] = {0x01, 0x02, 0x03};
    unsigned char *copy = (unsigned char *)aml_dup(buf, sizeof(buf));
    MACRO_ASSERT_TRUE(memcmp(buf, copy, sizeof(buf)) == 0);
    aml_free(copy);
}

MACRO_TEST(alloc_strdupa2_multiple_elements) {
    char *src[] = {"foo", "bar", "baz", NULL};
    char **dup = aml_strdupa2(src);
    for (int i=0; src[i]; i++)
        MACRO_ASSERT_TRUE(dup[i] == src[i]);
    aml_free(dup);
}

MACRO_TEST(alloc_strdupan_with_all_nulls) {
    char *src[] = {NULL, NULL, NULL};
    char **dup = aml_strdupan(src, 3);
    MACRO_ASSERT_TRUE(dup[0] == NULL);
    MACRO_ASSERT_TRUE(dup[1] == NULL);
    MACRO_ASSERT_TRUE(dup[2] == NULL);
    MACRO_ASSERT_TRUE(dup[3] == NULL);
    aml_free(dup);
}

MACRO_TEST(alloc_realloc_shrink_and_grow_loop) {
    char *p = (char *)aml_malloc(32);
    strcpy(p, "abc");
    for (int i = 0; i < 3; i++) {
        p = (char *)aml_realloc(p, 8);
        MACRO_ASSERT_STREQ(p, "abc");
        p = (char *)aml_realloc(p, 64);
        MACRO_ASSERT_STREQ(p, "abc");
    }
    aml_free(p);
}

MACRO_TEST(alloc_large_allocation) {
    size_t size = 1024 * 1024;
    char *p = (char *)aml_malloc(size);
    MACRO_ASSERT_TRUE(p != NULL);
    memset(p, 0xAB, size);
    for (size_t i = 0; i < size; i++)
        MACRO_ASSERT_TRUE((unsigned char)p[i] == 0xAB);
    aml_free(p);
}

MACRO_TEST(alloc_strdupf_varargs) {
    char *res = aml_strdupf("%s-%d", "id", 99);
    MACRO_ASSERT_STREQ(res, "id-99");
    aml_free(res);
}

MACRO_TEST(alloc_strdupan_partial_nulls) {
    char *src[] = {"A", NULL, "B", NULL, "C", NULL};
    char **dup = aml_strdupan(src, 6);
    MACRO_ASSERT_STREQ(dup[0], "A");
    MACRO_ASSERT_TRUE(dup[1] == NULL);
    MACRO_ASSERT_STREQ(dup[2], "B");
    MACRO_ASSERT_TRUE(dup[3] == NULL);
    MACRO_ASSERT_STREQ(dup[4], "C");
    MACRO_ASSERT_TRUE(dup[5] == NULL);
    MACRO_ASSERT_TRUE(dup[6] == NULL);
    aml_free(dup);
}

MACRO_TEST(alloc_realloc_ping_pong_many) {
    size_t cap = 1;
    unsigned char *p = (unsigned char*)aml_malloc(cap);
    for (int round=0; round<200; ++round) {
        cap += (round % 7) + 1;
        p = (unsigned char*)aml_realloc(p, cap);
        p[cap-1] = (unsigned char)(round & 0xFF);
        size_t new_cap = (cap > 5 ? cap - 5 : 1);
        p = (unsigned char*)aml_realloc(p, new_cap);
        cap = new_cap;
    }
    aml_free(p);
}

MACRO_TEST(alloc_strdupf_large_string) {
    char big[256];
    memset(big, 'A', sizeof(big)-1);
    big[sizeof(big)-1] = 0;

    char *s = aml_strdupf("prefix-%s-suffix-%d", big, 12345);
    MACRO_ASSERT_TRUE(strstr(s, "prefix-") == s);
    MACRO_ASSERT_TRUE(strstr(s, "-suffix-12345") != NULL);
    aml_free(s);
}

/* --- runner --- */
int main(void) {
    macro_test_case tests[64];
    size_t test_count = 0;

    MACRO_ADD(tests, alloc_malloc_zalloc_calloc_free);
    MACRO_ADD(tests, alloc_strdup_family_and_dup);
    MACRO_ADD(tests, alloc_realloc_grow_and_shrink_and_nullptr);
    MACRO_ADD(tests, alloc_strdupa_variants);
    MACRO_ADD(tests, alloc_dump_safe_to_call);
    MACRO_ADD(tests, alloc_zero_length_allocations);
    MACRO_ADD(tests, alloc_strdupa_variants_with_null_in_middle);
    MACRO_ADD(tests, alloc_strdupa2_null_array);
    MACRO_ADD(tests, alloc_free_nullptr_is_noop);
    MACRO_ADD(tests, alloc_realloc_null_and_zero);
    MACRO_ADD(tests, alloc_strdup_empty_and_onechar);
    MACRO_ADD(tests, alloc_strdupf_empty_format);
    MACRO_ADD(tests, alloc_strdupvf_variants);
    MACRO_ADD(tests, alloc_dup_binary_data);
    MACRO_ADD(tests, alloc_strdupa_empty_array);
    MACRO_ADD(tests, alloc_strdupan_handles_nulls_in_middle);
    MACRO_ADD(tests, alloc_strdupa2_empty_array);
    MACRO_ADD(tests, alloc_strdupa2_null_input);
    MACRO_ADD(tests, alloc_dup_and_free);
    MACRO_ADD(tests, alloc_strdupa2_multiple_elements);
    MACRO_ADD(tests, alloc_strdupan_with_all_nulls);
    MACRO_ADD(tests, alloc_realloc_shrink_and_grow_loop);
    MACRO_ADD(tests, alloc_large_allocation);
    MACRO_ADD(tests, alloc_strdupf_varargs);
    MACRO_ADD(tests, alloc_strdupan_partial_nulls);
    MACRO_ADD(tests, alloc_realloc_ping_pong_many);
    MACRO_ADD(tests, alloc_strdupf_large_string);

    macro_run_all("a-memory-library/aml_alloc", tests, test_count);
    return 0;
}
