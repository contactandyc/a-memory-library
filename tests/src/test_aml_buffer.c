// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

// test_aml_buffer.c
#include "the-macro-library/macro_test.h"
#include "a-memory-library/aml_buffer.h"
#include "a-memory-library/aml_pool.h"
#include "a-memory-library/aml_alloc.h"

#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#define SAFE_FREE_HEAP_PTR(p) do { if (p) aml_free(p); } while (0)

static void buffer_setvf_helper(aml_buffer_t *b, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    aml_buffer_setvf(b, fmt, args);
    va_end(args);
}

MACRO_TEST(buffer_init_zero_and_append) {
    aml_buffer_t *b = aml_buffer_init(0);
    MACRO_ASSERT_TRUE(b != NULL);

    aml_buffer_appends(b, "he");
    aml_buffer_appends(b, "llo");
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 5);
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "hello");

    aml_buffer_appendc(b, '!');
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "hello!");

    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_sets_setc_setn_setf_setvf) {
    aml_buffer_t *b = aml_buffer_init(8);

    aml_buffer_sets(b, "abc");
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 3);
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "abc");

    aml_buffer_setc(b, 'Z');
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 1);
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "Z");

    aml_buffer_setn(b, 'x', 5);
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 5);
    MACRO_ASSERT_TRUE(memcmp(aml_buffer_data(b), "xxxxx", 5) == 0);

    aml_buffer_setf(b, "val=%d %s", 42, "ok");
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "val=42 ok");

    buffer_setvf_helper(b, "pi=%.2f", 3.14159);
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "pi=3.14");

    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_resize_shrink_end) {
    aml_buffer_t *b = aml_buffer_init(8);
    aml_buffer_sets(b, "abc");
    (void)aml_buffer_resize(b, 6);
    char *d = aml_buffer_data(b);
    d[3]='X'; d[4]='Y'; d[5]='Z';
    MACRO_ASSERT_TRUE(memcmp(aml_buffer_data(b), "abcXYZ", 6) == 0);

    (void)aml_buffer_shrink_by(b, 2);
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 4);
    MACRO_ASSERT_TRUE(memcmp(aml_buffer_data(b), "abcX", 4) == 0);

    MACRO_ASSERT_TRUE(aml_buffer_end(b) == aml_buffer_data(b) + aml_buffer_length(b));
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_append_alloc_and_ualloc_alignment) {
    aml_buffer_t *b = aml_buffer_init(1);
    void *w = aml_buffer_append_alloc(b, 5);
    MACRO_ASSERT_TRUE((((uintptr_t)w) & 7) == 0);
    memcpy(w, "ABCDE", 5);
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 5);
    MACRO_ASSERT_TRUE(memcmp(aml_buffer_data(b), "ABCDE", 5) == 0);

    char *w2 = (char*)aml_buffer_append_ualloc(b, 3);
    w2[0]='x'; w2[1]='y'; w2[2]='z';
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 8);
    MACRO_ASSERT_TRUE(memcmp(aml_buffer_data(b), "ABCDExyz", 8) == 0);
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_appendsz_and_appendf) {
    aml_buffer_t *b = aml_buffer_init(0);
    aml_buffer_appends(b, "hi");
    size_t before = aml_buffer_length(b);
    aml_buffer_appendsz(b, "xx");
    size_t after = aml_buffer_length(b);
    MACRO_ASSERT_EQ_SZ(after, before + 3);
    MACRO_ASSERT_TRUE(aml_buffer_data(b)[after] == '\0');

    aml_buffer_clear(b);
    aml_buffer_appendf(b, "num=%d %s", 7, "ok");
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "num=7 ok");
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_alloc_non_preserving_and_reset_heap_shrink) {
    aml_buffer_t *b = aml_buffer_init(8);
    aml_buffer_sets(b, "abc");
    (void)aml_buffer_alloc(b, 10);
    memset(aml_buffer_data(b), 'Q', 10);
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 10);

    aml_buffer_appends(b, "0123456789ABCDEFGHIJ");
    aml_buffer_reset(b, 16);
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 0);
    aml_buffer_appends(b, "ok");
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "ok");
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_detach_heap_owned_and_pool_owned) {
    aml_buffer_t *bh = aml_buffer_init(0);
    aml_buffer_appends(bh, "world");
    size_t lenh = 0;
    char *owned = aml_buffer_detach(bh, &lenh);
    MACRO_ASSERT_EQ_SZ(lenh, 5);
    MACRO_ASSERT_TRUE(memcmp(owned, "world", 5) == 0);
    SAFE_FREE_HEAP_PTR(owned);
    aml_buffer_destroy(bh);

    aml_pool_t *p = aml_pool_init(512);
    aml_buffer_t *bp = aml_buffer_pool_init(p, 32);
    aml_buffer_appends(bp, "pool");
    size_t lenp = 0;
    char *owned_p = aml_buffer_detach(bp, &lenp);
    MACRO_ASSERT_EQ_SZ(lenp, 4);
    MACRO_ASSERT_TRUE(memcmp(owned_p, "pool", 4) == 0);
    aml_pool_destroy(p);
}

MACRO_TEST(buffer_appendvf_exact_fit_and_multi_grow) {
    aml_buffer_t *b = aml_buffer_init(4);
    aml_buffer_setf(b, "%s", "abcd");
    aml_buffer_appendf(b, "%s", "");
    for (int i=0;i<6;i++) aml_buffer_appendc(b, 'X');
    aml_buffer_appendf(b, "-%s-%d", "grow", 123);
    MACRO_ASSERT_TRUE(strstr(aml_buffer_data(b), "grow") != NULL);
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_append_alloc_adds_padding_to_8_bytes) {
    aml_buffer_t *b = aml_buffer_init(1);
    memcpy(aml_buffer_append_ualloc(b, 3), "abc", 3);
    void *p = aml_buffer_append_alloc(b, 5);
    MACRO_ASSERT_TRUE((((uintptr_t)p) & 7) == 0);
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_reset_on_pool_backed_does_not_free) {
    aml_pool_t *pool = aml_pool_init(256);
    aml_buffer_t *b = aml_buffer_pool_init(pool, 8);
    for (int i=0;i<50;i++) aml_buffer_appends(b, "abcdefghij");
    aml_buffer_reset(b, 16);
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 0);
    aml_buffer_appends(b, "ok");
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "ok");
    aml_pool_destroy(pool);
}

MACRO_TEST(buffer_detach_empty_and_reuse_object) {
    aml_buffer_t *b = aml_buffer_init(0);
    size_t out = 1234;
    char *d = aml_buffer_detach(b, &out);
    MACRO_ASSERT_EQ_SZ(out, 0);
    SAFE_FREE_HEAP_PTR(d);
    aml_buffer_appends(b, "reused");
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "reused");
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_appendn_nonpositive_noop) {
    aml_buffer_t *b = aml_buffer_init(4);
    aml_buffer_sets(b, "base");
    aml_buffer_appendn(b, 'X', 0);
    aml_buffer_appendn(b, 'X', -5);
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "base");
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_sets_overwrites_previous_content) {
    aml_buffer_t *b = aml_buffer_init(8);
    for (int i=0;i<10;i++) aml_buffer_appends(b, "1234567890");
    aml_buffer_sets(b, "OK");
    MACRO_ASSERT_EQ_SZ(aml_buffer_length(b), 2);
    MACRO_ASSERT_STREQ(aml_buffer_data(b), "OK");
    aml_buffer_destroy(b);
}

/* Extra Stress / Edge-Case Tests */
MACRO_TEST(buffer_grow_and_shrink_cycles) {
    aml_buffer_t *b = aml_buffer_init(1);
    for (int i=0; i<100; i++) {
        aml_buffer_appendn(b, 'A' + (i % 26), (i % 20) + 1);
        aml_buffer_shrink_by(b, (i % 10));
    }
    MACRO_ASSERT_TRUE(aml_buffer_length(b) > 0);
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_large_appends) {
    aml_buffer_t *b = aml_buffer_init(8);
    char block[1024];
    memset(block, 'Z', sizeof(block));
    for (int i=0; i<200; i++) aml_buffer_append(b, block, sizeof(block));
    MACRO_ASSERT_TRUE(aml_buffer_length(b) >= 200*1024);
    aml_buffer_destroy(b);
}

MACRO_TEST(buffer_append_binary_with_nulls) {
    aml_buffer_t *b = aml_buffer_init(8);
    unsigned char data[6] = { 'A', 0x00, 'B', 0x00, 'C', 0x00 };
    aml_buffer_append(b, data, sizeof(data));
    MACRO_ASSERT_TRUE(memcmp(aml_buffer_data(b), data, sizeof(data)) == 0);
    aml_buffer_destroy(b);
}

/* --- runner --- */
int main(void) {
    macro_test_case tests[128];
    size_t test_count = 0;

    MACRO_ADD(tests, buffer_init_zero_and_append);
    MACRO_ADD(tests, buffer_sets_setc_setn_setf_setvf);
    MACRO_ADD(tests, buffer_resize_shrink_end);
    MACRO_ADD(tests, buffer_append_alloc_and_ualloc_alignment);
    MACRO_ADD(tests, buffer_appendsz_and_appendf);
    MACRO_ADD(tests, buffer_alloc_non_preserving_and_reset_heap_shrink);
    MACRO_ADD(tests, buffer_detach_heap_owned_and_pool_owned);
    MACRO_ADD(tests, buffer_appendvf_exact_fit_and_multi_grow);
    MACRO_ADD(tests, buffer_append_alloc_adds_padding_to_8_bytes);
    MACRO_ADD(tests, buffer_reset_on_pool_backed_does_not_free);
    MACRO_ADD(tests, buffer_detach_empty_and_reuse_object);
    MACRO_ADD(tests, buffer_appendn_nonpositive_noop);
    MACRO_ADD(tests, buffer_sets_overwrites_previous_content);
    MACRO_ADD(tests, buffer_grow_and_shrink_cycles);
    MACRO_ADD(tests, buffer_large_appends);
    MACRO_ADD(tests, buffer_append_binary_with_nulls);

    macro_run_all("a-memory-library/aml_buffer", tests, test_count);
    return 0;
}
