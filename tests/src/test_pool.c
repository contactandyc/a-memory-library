#include "a-memory-library/aml_pool.h"

int main() {
    aml_pool_t *pool = aml_pool_init(1024 * 1024);
    char *s = aml_pool_strdup(pool, "Hello, World!");
    printf("%s\n", s);
    aml_pool_clear(pool);
    // aml_pool_destroy(pool);
    return 0;
}