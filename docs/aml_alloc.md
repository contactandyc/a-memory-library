# AML Memory Management Functions ([aml_alloc.h](../include/a-memory-library/aml_alloc.h))

### malloc, calloc, zalloc, and realloc

#### `void* aml_malloc(size_t len)`

- **Description**: Allocates `len` bytes of uninitialized memory.
- **Parameters**: `len` - Number of bytes to allocate.
- **Return**: Pointer to the allocated memory.

#### `void* aml_calloc(size_t num_items, size_t size)`

- **Description**: Allocates memory for an array of `num_items`, each of `size` bytes, and initializes all bytes to zero.
- **Parameters**: `num_items` - Number of items, `size` - Size of each item.
- **Return**: Pointer to the allocated memory.

#### `void* aml_zalloc(size_t len)`

- **Description**: Allocates `len` bytes of memory initialized to zero.
- **Parameters**: `len` - Number of bytes to allocate.
- **Return**: Pointer to the allocated memory.

#### `void* aml_realloc(void* p, size_t len)`

- **Description**: Reallocates the memory block pointed to by `p`, making it `len` bytes long.
- **Parameters**: `p` - Original pointer, `len` - New size.
- **Return**: Pointer to the reallocated memory.

### Free memory

#### `void aml_free(void* p)`

- **Description**: Frees the memory space pointed to by `p`.
- **Parameters**: `p` - Pointer to the memory to free.

### strdup and dup (similar to memdup)

#### `char* aml_strdup(const char* p)`

- **Description**: Duplicates a string pointed to by `p`.
- **Parameters**: `p` - Pointer to the original string.
- **Return**: Pointer to the duplicated string.

#### `char* aml_strdupf(const char* p, ...)`

- **Description**: Creates a new string using format specifiers.
- **Parameters**: `p` - Format string, `...` - Additional arguments.
- **Return**: Pointer to the new string.

#### `char* aml_strdupvf(const char* p, va_list args)`

- **Description**: Similar to `aml_strdupf` but takes a `va_list` for format arguments.
- **Parameters**: `p` - Format string, `args` - `va_list` containing arguments.
- **Return**: Pointer to the new string.

#### `void* aml_dup(const void* p, size_t len)`

- **Description**: Duplicates a memory block.
- **Parameters**: `p` - Pointer to the original memory block, `len` - Number of bytes to duplicate.
- **Return**: Pointer to the new memory block.

### Duplicate arrays of strings

#### `char** aml_strdupa(char** p)`

- **Description**: Duplicates a NULL terminated array of strings as a single allocation
- **Parameters**: `p` - Pointer to the original NULL terminated array of strings.
- **Return**: Pointer to the duplicated NULL terminated array of strings.

#### `char** aml_strdupan(char** p, size_t n)`

- **Description**: The same as `aml_strdupa` except the array of strings length is determined by `n`
- **Parameters**: `p` - Pointer to the array of strings, `n` - Number of strings in p
- **Return**: Pointer to the duplicated NULL terminated array of strings.

#### `char** aml_strdupa2(char** p)`

- **Description**: The same as `aml_strdupa`, except only the pointers are duplicated and not the actual strings.
- **Parameters**: `p` - Pointer to the original NULL terminated array of strings.
- **Return**: Pointer to the duplicated NULL terminated array of strings.

## Usage Example

```c
#include "a-memory-library/aml_memory.h"

void example_usage() {
  char *str = aml_malloc(50);
  char *copy = aml_strdup("Hello, world!");
  aml_free(str);
  aml_free(copy);
}
```
