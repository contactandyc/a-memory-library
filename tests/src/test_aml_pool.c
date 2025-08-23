// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

// test_aml_pool.c
#include "the-macro-library/macro_test.h"
#include "a-memory-library/aml_pool.h"
#include "a-memory-library/aml_alloc.h"

#include <string.h>
#include <stdint.h>
#include <stdarg.h>

static char *pool_vdup(aml_pool_t *pool, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *s = aml_pool_strdupvf(pool, fmt, args);
    va_end(args);
    return s;
}

MACRO_TEST(pool_basic_alloc_ualloc_zalloc_alignment) {
    aml_pool_t *p = aml_pool_init(256);

    // aligned allocation
    uint32_t *u = (uint32_t*)aml_pool_alloc(p, sizeof(uint32_t));
    *u = 0xDEADBEEF;
    MACRO_ASSERT_EQ_INT(*u, (int)0xDEADBEEF);

    // unaligned allocation (works but not necessarily aligned—API guarantees are different)
    char *ua = (char*)aml_pool_ualloc(p, 3);
    ua[0]='a'; ua[1]='b'; ua[2]='c';

    // zalloc should be zeroed
    size_t N = 17;
    unsigned char *z = (unsigned char*)aml_pool_zalloc(p, N);
    for (size_t i=0;i<N;i++) MACRO_ASSERT_EQ_INT(z[i], 0);

    // calloc(num,size)
    int *c = (int*)aml_pool_calloc(p, 10, sizeof(int));
    for (int i=0;i<10;i++) MACRO_ASSERT_EQ_INT(c[i], 0);

    aml_pool_destroy(p);
}

MACRO_TEST(pool_aalloc_alignment_64) {
    aml_pool_t *p = aml_pool_init(256);
    void *mem = aml_pool_aalloc(p, 64, 100);
    MACRO_ASSERT_TRUE(((uintptr_t)mem & 63) == 0);
    aml_pool_destroy(p);
}

MACRO_TEST(pool_min_max_alloc_behaviour) {
    aml_pool_t *p = aml_pool_init(128);
    size_t rlen = 0;
    char *blk = (char*)aml_pool_min_max_alloc(p, &rlen, 10, 50);
    MACRO_ASSERT_TRUE(blk != NULL);
    MACRO_ASSERT_TRUE(rlen >= 10 && rlen <= 50);
    // write rlen bytes
    for (size_t i=0;i<rlen;i++) blk[i] = (char)('a' + (i%26));
    aml_pool_destroy(p);
}

MACRO_TEST(pool_save_restore_stack_like) {
    aml_pool_t *p = aml_pool_init(128);

    aml_pool_marker_t m;
    aml_pool_save(p, &m);

    char *a = (char*)aml_pool_alloc(p, 32);
    strcpy(a, "temp-data");
    char *b = (char*)aml_pool_alloc(p, 32);
    strcpy(b, "more-temp");

    // restore should drop a and b
    aml_pool_restore(p, &m);

    // new allocation should reuse the same block without leaking
    char *c = (char*)aml_pool_alloc(p, 32);
    strcpy(c, "new-data");
    MACRO_ASSERT_STREQ(c, "new-data");

    aml_pool_destroy(p);
}

MACRO_TEST(pool_clear_reuse_after_growth) {
    aml_pool_t *p = aml_pool_init(128);

    // force growth beyond initial block
    for (int i=0;i<50;i++) (void)aml_pool_alloc(p, 64);

    size_t used_before = aml_pool_used(p);
    aml_pool_clear(p);
    size_t used_after = aml_pool_used(p);

    // After clear, pool should be back to baseline (<= previous used)
    MACRO_ASSERT_TRUE(used_after <= used_before);

    // And it should still be usable
    char *d = (char*)aml_pool_alloc(p, 32);
    strcpy(d, "ok");
    MACRO_ASSERT_STREQ(d, "ok");

    aml_pool_destroy(p);
}

MACRO_TEST(pool_strdup_family) {
    aml_pool_t *p = aml_pool_init(256);
    char *s1 = aml_pool_strdup(p, "hello");
    MACRO_ASSERT_STREQ(s1, "hello");

    char *s2 = aml_pool_strdupf(p, "num=%d %s", 9, "go");
    MACRO_ASSERT_STREQ(s2, "num=9 go");

    // Proper strdupvf via helper
    char *s2v = pool_vdup(p, "v=%d %s", 12, "ok");
    MACRO_ASSERT_STREQ(s2v, "v=12 ok");

    char *s3 = aml_pool_strndup(p, "abcdef", 3);
    MACRO_ASSERT_STREQ(s3, "abc");

    const unsigned char raw[] = {1,2,3,4};
    unsigned char *dup = (unsigned char*)aml_pool_dup(p, raw, sizeof(raw));
    MACRO_ASSERT_TRUE(memcmp(dup, raw, sizeof(raw)) == 0);

    unsigned char *udup = (unsigned char*)aml_pool_udup(p, raw, sizeof(raw));
    MACRO_ASSERT_TRUE(memcmp(udup, raw, sizeof(raw)) == 0);

    aml_pool_destroy(p);
}


MACRO_TEST(pool_split_variants_basic) {
    aml_pool_t *p = aml_pool_init(256);
    size_t n = 0;

    char **s = aml_pool_split(p, &n, ',', "a,b,,c,");
    // should include empty segments
    MACRO_ASSERT_TRUE(n == 5);
    MACRO_ASSERT_STREQ(s[0], "a");
    MACRO_ASSERT_STREQ(s[1], "b");
    MACRO_ASSERT_STREQ(s[2], "");
    MACRO_ASSERT_STREQ(s[3], "c");
    MACRO_ASSERT_STREQ(s[4], "");
    MACRO_ASSERT_TRUE(s[5] == NULL);

    char **s2 = aml_pool_split2(p, &n, ',', "a,b,,c,");
    // should exclude empty segments
    MACRO_ASSERT_TRUE(n == 3);
    MACRO_ASSERT_STREQ(s2[0], "a");
    MACRO_ASSERT_STREQ(s2[1], "b");
    MACRO_ASSERT_STREQ(s2[2], "c");
    MACRO_ASSERT_TRUE(s2[3] == NULL);

    char **sf = aml_pool_splitf(p, &n, ':', "%s:%d::%s", "x", 42, "y");
    MACRO_ASSERT_TRUE(n == 4);
    MACRO_ASSERT_STREQ(sf[0], "x");
    MACRO_ASSERT_STREQ(sf[1], "42");
    MACRO_ASSERT_STREQ(sf[2], "");
    MACRO_ASSERT_STREQ(sf[3], "y");
    MACRO_ASSERT_TRUE(sf[4] == NULL);

    char **s2f = aml_pool_split2f(p, &n, ':', "%s::%s", "p", "q");
    MACRO_ASSERT_TRUE(n == 2);
    MACRO_ASSERT_STREQ(s2f[0], "p");
    MACRO_ASSERT_STREQ(s2f[1], "q");
    MACRO_ASSERT_TRUE(s2f[2] == NULL);

    aml_pool_destroy(p);
}

MACRO_TEST(pool_split_with_escape_variants) {
    aml_pool_t *p = aml_pool_init(512);
    size_t n = 0;

    // escaped commas should stay in token
    const char *in = "a\\,b,c\\\\,d\\,\\,e";
    // tokens: ["a,b", "c\\", "d,,e"]
    char **t = aml_pool_split_with_escape(p, &n, ',', '\\', in);
    MACRO_ASSERT_TRUE(n == 3);
    MACRO_ASSERT_STREQ(t[0], "a,b");
    MACRO_ASSERT_STREQ(t[1], "c\\");
    MACRO_ASSERT_STREQ(t[2], "d,,e");
    MACRO_ASSERT_TRUE(t[3] == NULL);

    // same but drop empty via *2
    const char *in2 = "\\,x,,\\,y,";
    // raw split_with_escape => [",x", "", ",y", ""]
    char **t2 = aml_pool_split_with_escape2(p, &n, ',', '\\', in2);
    // dropping empties => [",x", ",y"]
    MACRO_ASSERT_TRUE(n == 2);
    MACRO_ASSERT_STREQ(t2[0], ",x");
    MACRO_ASSERT_STREQ(t2[1], ",y");
    MACRO_ASSERT_TRUE(t2[2] == NULL);

    aml_pool_destroy(p);
}

MACRO_TEST(pool_strdupa_families) {
    aml_pool_t *p = aml_pool_init(256);

    char *arr[] = {"a", "bb", NULL};
    char **dup1 = aml_pool_strdupa(p, arr);
    MACRO_ASSERT_STREQ(dup1[0], "a");
    MACRO_ASSERT_STREQ(dup1[1], "bb");
    MACRO_ASSERT_TRUE(dup1[2] == NULL);

    char *arr2[] = {"x", NULL, "y", NULL};
    char **dup2 = aml_pool_strdupan(p, arr2, 4);
    MACRO_ASSERT_STREQ(dup2[0], "x");
    MACRO_ASSERT_TRUE(dup2[1] == NULL);
    MACRO_ASSERT_STREQ(dup2[2], "y");
    MACRO_ASSERT_TRUE(dup2[3] == NULL);
    MACRO_ASSERT_TRUE(dup2[4] == NULL);


    char *arr3[] = {"x", "y", NULL};
    char **dup3 = aml_pool_strdupa2(p, arr3);
    MACRO_ASSERT_TRUE(dup3 != arr3);
    MACRO_ASSERT_TRUE(dup3[0] == arr3[0]);
    MACRO_ASSERT_TRUE(dup3[1] == arr3[1]);
    MACRO_ASSERT_TRUE(dup3[2] == NULL);

    aml_pool_destroy(p);
}

MACRO_TEST(pool_base64_roundtrip) {
    aml_pool_t *p = aml_pool_init(512);

    const unsigned char bin[] = {0x00, 0xFF, 0x10, 0x7E, 0x80, 0xAA};
    char *b64 = aml_pool_base64_encode(p, bin, sizeof(bin));
    size_t out_len = 0;
    unsigned char *rt = aml_pool_base64_decode(p, &out_len, b64);

    MACRO_ASSERT_EQ_SZ(out_len, sizeof(bin));
    MACRO_ASSERT_TRUE(memcmp(bin, rt, sizeof(bin)) == 0);

    // empty inputs behave
    char *b64e = aml_pool_base64_encode(p, (const unsigned char*)"", 0);
    MACRO_ASSERT_STREQ(b64e, "");
    (void)aml_pool_base64_decode(p, &out_len, "");
    MACRO_ASSERT_EQ_SZ(out_len, 0);

    aml_pool_destroy(p);
}

MACRO_TEST(pool_subpool_lifecycle) {
    aml_pool_t *root = aml_pool_init(1024);
    aml_pool_t *sub = aml_pool_pool_init(root, 128);

    char *r = (char*)aml_pool_alloc(root, 32);
    strcpy(r, "root");

    char *s = (char*)aml_pool_alloc(sub, 32);
    strcpy(s, "sub");

    MACRO_ASSERT_STREQ(r, "root");
    MACRO_ASSERT_STREQ(s, "sub");

    // Clear only sub; root memory should remain
    aml_pool_clear(sub);
    MACRO_ASSERT_STREQ(r, "root");

    // Destroy root last
    aml_pool_destroy(root);
}

MACRO_TEST(pool_strdupa_empty_array) {
    aml_pool_t *p = aml_pool_init(64);
    char *arr[] = { NULL };
    char **dup = aml_pool_strdupa(p, arr);
    MACRO_ASSERT_TRUE(dup[0] == NULL);
    aml_pool_destroy(p);
}

/* --- runner --- */
int main(void) {
    macro_test_case tests[128];
    size_t test_count = 0;

    MACRO_ADD(tests, pool_basic_alloc_ualloc_zalloc_alignment);
    MACRO_ADD(tests, pool_aalloc_alignment_64);
    MACRO_ADD(tests, pool_min_max_alloc_behaviour);
    MACRO_ADD(tests, pool_save_restore_stack_like);
    MACRO_ADD(tests, pool_clear_reuse_after_growth);
    MACRO_ADD(tests, pool_strdup_family);
    MACRO_ADD(tests, pool_split_variants_basic);
    MACRO_ADD(tests, pool_split_with_escape_variants);
    MACRO_ADD(tests, pool_strdupa_families);
    MACRO_ADD(tests, pool_base64_roundtrip);
    MACRO_ADD(tests, pool_subpool_lifecycle);
    MACRO_ADD(tests, pool_strdupa_empty_array);


    macro_run_all("a-memory-library/aml_pool", tests, test_count);
    return 0;
}
