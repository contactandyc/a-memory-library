# A memory library

# AML Memory Management Functions (aml_alloc.h)

These functions are defined as macros, but their underlying types are also described for clarity.

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


# AML Pool Memory Management (aml_pool.h)

The `aml_pool` offers an API similar to standard memory allocation functions (like malloc and calloc) but with the added advantage of customized memory pooling. This approach avoids the need for individual free operations, instead opting for pool-level memory management. It's tailored for high-performance scenarios where memory fragmentation and allocation overhead are critical concerns.

### Initialization, Clear, and Destruction
The pool is initialized with a size, optionally from another pool, can be cleared and destroyed 

#### `aml_pool_t* aml_pool_init(size_t size)`

- **Description**: Initializes a memory pool with the given size.
- **Parameters**: `size` - The size of the initial memory pool.
- **Return**: A pointer to the initialized memory pool.

#### `aml_pool_t* aml_pool_pool_init(aml_pool_t *pool, size_t initial_size)`

- **Description**: Creates a sub-pool from an existing pool with a specified initial size.
- **Parameters**: `pool` - Pointer to the existing pool, `initial_size` - Size of the new sub-pool.
- **Return**: Pointer to the newly created sub-pool.

#### `void aml_pool_clear(aml_pool_t *h)`

- **Description**: Clears the memory pool, making all allocated memory reusable.
- **Parameters**: `h` - Pointer to the memory pool.

#### `void aml_pool_destroy(aml_pool_t *h)`

- **Description**: Destroys the memory pool, freeing up all associated memory.
- **Parameters**: `h` - Pointer to the memory pool to be destroyed.

### Pool stats

#### `size_t aml_pool_size(aml_pool_t *h)`

- **Description**: Returns the total number of bytes allocated from the pool through various allocation calls.
- **Parameters**: `h` - Pointer to the memory pool.
- **Return**: Total number of bytes allocated from the pool.

#### `size_t aml_pool_used(aml_pool_t *h)`

- **Description**: Provides the total number of bytes allocated by the pool itself, which includes overhead for pool structures. This value is always greater than the total allocated by user calls (`aml_pool_size`).
- **Parameters**: `h` - Pointer to the memory pool.
- **Return**: Total number of bytes used by the pool, including overhead.

### Controlling Pool Growth

#### `void aml_pool_set_minimum_growth_size(aml_pool_t *h, size_t size)`

- **Description**: Sets the minimum size for additional memory blocks allocated when the pool grows. This is beneficial when the anticipated excess in pool size is minimal, preventing the default behavior of doubling the pool's memory usage.
- **Parameters**: `h` - Pointer to the memory pool, `size` - Minimum size for growth blocks.

### Alloc (similar to malloc), calloc, zalloc, ualloc, min_max_alloc

#### `void* aml_pool_alloc(aml_pool_t *h, size_t len)`

- **Description**: Allocates `len` bytes of uninitialized memory from the pool, ensuring the memory is aligned.
- **Parameters**: `h` - Pointer to the memory pool, `len` - Number of bytes to allocate.
- **Return**: Pointer to the allocated memory.

#### `void* aml_pool_calloc(aml_pool_t *h, size_t num_items, size_t size)`

- **Description**: Allocates memory for an array of `num_items`, each of `size` bytes, from the pool and initializes all bytes to zero, ensuring the memory is aligned.
- **Parameters**: `h` - Pointer to the memory pool, `num_items` - Number of items, `size` - Size of each item.
- **Return**: Pointer to the zero-initialized memory.

#### `void* aml_pool_zalloc(aml_pool_t *h, size_t len)`

- **Description**: Allocates `len` bytes of memory from the pool and initializes all bytes to zero, ensuring the memory is aligned.
- **Parameters**: `h` - Pointer to the memory pool, `len` - Number of bytes to allocate.
- **Return**: Pointer to the zero-initialized memory.

#### `void* aml_pool_min_max_alloc(aml_pool_t *h, size_t *rlen, size_t min_len, size_t len)`

- **Description**: Allocates memory with a size between `min_len` and `len` bytes. The actual length of the allocated memory is returned in `rlen`.
- **Parameters**: `h` - Pointer to the memory pool, `rlen` - Pointer to store the actual allocated size, `min_len` - Minimum number of bytes, `len` - Maximum number of bytes.
- **Return**: Pointer to the allocated memory.

#### `void* aml_pool_ualloc(aml_pool_t *h, size_t len)`

- **Description**: Allocates `len` bytes of uninitialized memory from the pool without ensuring alignment.
- **Parameters**: `h` - Pointer to the memory pool, `len` - Number of bytes to allocate.
- **Return**: Pointer to the allocated memory.

### strdup and dup functions

#### `char* aml_pool_strdup(aml_pool_t *h, const char* p)`

- **Description**: Duplicates a string `p` using the memory pool. The allocated memory for the duplicated string will be unaligned. For aligned memory, consider using `aml_pool_dup`.
- **Parameters**: `h` - Pointer to the memory pool, `p` - String to duplicate.
- **Return**: Pointer to the duplicated string.

#### `char* aml_pool_strdupf(aml_pool_t *h, const char *p, ...)`

- **Description**: Allocates a copy of the formatted string specified by `p` using the memory pool, similar to the standard `sprintf` function.
- **Parameters**: `h` - Pointer to the memory pool, `p` - Format string, `...` - Additional format arguments.
- **Return**: Pointer to the formatted string.

#### `char* aml_pool_strdupvf(aml_pool_t *h, const char *p, va_list args)`

- **Description**: Similar to `aml_pool_strdupf`, but takes a `va_list` for the format arguments. Useful for extending objects that use the pool as their memory base.
- **Parameters**: `h` - Pointer to the memory pool, `p` - Format string, `args` - `va_list` containing format arguments.
- **Return**: Pointer to the formatted string.

#### `char* aml_pool_strndup(aml_pool_t *h, const char *p, size_t length)`

- **Description**: Duplicates a string `p` with a maximum length of `length` bytes using the memory pool. The string will be null-terminated.
- **Parameters**: `h` - Pointer to the memory pool, `p` - String to duplicate, `length` - Maximum length of the string.
- **Return**: Pointer to the duplicated string.

#### `void* aml_pool_dup(aml_pool_t *h, const void *data, size_t len)`

- **Description**: Allocates a copy of the data block `data` with a length of `len` bytes, ensuring the memory is aligned.
- **Parameters**: `h` - Pointer to the memory pool, `data` - Data to duplicate, `len` - Length of the data.
- **Return**: Pointer to the duplicated data.

#### `void* aml_pool_udup(aml_pool_t *h, const void *data, size_t len)`

### Split Functions

- **Description**: Similar to `aml_pool_dup`, but the allocated memory for the duplicated data will be unaligned.
- **Parameters**: `h` - Pointer to the memory pool, `data` - Data to duplicate, `len` - Length of the data.
- **Return**: Pointer to the duplicated data.

#### `char** aml_pool_split(aml_pool_t *h, size_t *num_splits, char delim, const char *p)`

- **Description**: Splits a string `p` into an array of strings using `delim` as the delimiter. The resulting array will be NULL-terminated. If `p` is NULL, the array will consist of a single NULL string.
- **Parameters**: `h` - Pointer to the memory pool, `num_splits` - Pointer to store the number of splits (can be NULL), `delim` - Delimiter character, `p` - String to split.
- **Return**: Array of split strings.

#### `char** aml_pool_splitf(aml_pool_t *h, size_t *num_splits, char delim, const char *p, ...)`

- **Description**: Similar to `aml_pool_split`, but allows for a formatted input string. It splits the formatted string into an array of strings using `delim`.
- **Parameters**: `h` - Pointer to the memory pool, `num_splits` - Pointer to store the number of splits (can be NULL), `delim` - Delimiter character, `p` - Format string, `...` - Additional format arguments.
- **Return**: Array of split strings.

#### `char** aml_pool_split2(aml_pool_t *h, size_t *num_splits, char delim, const char *p)`

- **Description**: Similar to `aml_pool_split`, but excludes empty strings from the result.
- **Parameters**: `h` - Pointer to the memory pool, `num_splits` - Pointer to store the number of splits (can be NULL), `delim` - Delimiter character, `p` - String to split.
- **Return**: Array of split strings, excluding empty strings.

#### `char** aml_pool_split2f(aml_pool_t *h, size_t *num_splits, char delim, const char *p, ...)`

- **Description**: Combines the functionalities of `aml_pool_splitf` and `aml_pool_split2`, allowing for formatted input string and excluding empty strings from the result.
- **Parameters**: `h` - Pointer to the memory pool, `num_splits` - Pointer to store the number of splits (can be NULL), `delim` - Delimiter character, `p` - Format string, `...` - Additional format arguments.
- **Return**: Array of split strings, excluding empty strings.

### Array Duplication Functions

#### `char** aml_pool_strdupa(aml_pool_t *pool, char **arr)`

- **Description**: Duplicates an entire array of strings `arr`, including the NULL terminator, using the memory pool.
- **Parameters**: `pool` - Pointer to the memory pool, `arr` - Array of strings to duplicate.
- **Return**: Duplicated array of strings.

#### `char** aml_pool_strdupan(aml_pool_t *pool, char **arr, size_t num)`

- **Description**: Similar to `aml_pool_strdupa`, but duplicates only the first `num` strings of the array `arr`.
- **Parameters**: `pool` - Pointer to the memory pool, `arr` - Array of strings to duplicate, `num` - Number of strings to duplicate.
- **Return**: Duplicated array of strings.

#### `char** aml_pool_strdupa2(aml_pool_t *pool, char **arr)`

- **Description**: Duplicates the structure of an array of strings `arr` (the array of pointers), but not the strings themselves, using the memory pool.
- **Parameters**: `pool` - Pointer to the memory pool, `arr` - Array of strings whose structure is to be duplicated.
- **Return**: Duplicated array of pointers.

## Usage Example

```c
#include "a-memory-library/aml_pool.h"

void example_usage() {
  aml_pool_t *pool = aml_pool_init(1024);
  
  char *str = aml_pool_strdup(pool, "Hello, world!");
  char *str2 = aml_pool_strdupf(pool, "Hello, %s!", "world");
  printf( "%s == %s\n", str, str2 );
  printf( "pool size: %zu\n", aml_pool_size(pool) );
  aml_pool_clear(pool);  // Clear the pool for reuse
  // overwrites str above!
  char *str3 = aml_pool_strdup(pool, "Goodbye, world!");
    
  aml_pool_destroy(pool); 
}
```


# AML Buffer Management (aml_buffer.h)

The `aml_buffer` component of the AML Memory Library is a dynamic buffer management system, designed to handle both string and binary data. It provides a set of functionalities similar to the C++ string class but is optimized for performance and flexibility in managing various types of data.

### Initialization and Destruction

#### `aml_buffer_t* aml_buffer_init(size_t size)`

- **Description**: Initializes a buffer with a specified initial size. The buffer will auto-resize as needed. Specifying an efficient initial size can enhance performance.
- **Parameters**: `size` - The initial size of the buffer.
- **Return**: Pointer to the initialized buffer.

#### `aml_buffer_t* aml_buffer_pool_init(aml_pool_t *pool, size_t initial_size)`

- **Description**: Similar to `aml_buffer_init`, but allocates the buffer using a specified memory pool. This eliminates the need for explicit destruction.
- **Parameters**: `pool` - Memory pool for allocation, `initial_size` - Initial size of the buffer.
- **Return**: Pointer to the initialized buffer.

#### `void aml_buffer_destroy(aml_buffer_t *h)`

- **Description**: Destroys the buffer, freeing all associated resources.
- **Parameters**: `h` - Pointer to the buffer

### Clear and Resize
Functions to resize the buffer and manage its memory allocation.

####  `void aml_buffer_clear(aml_buffer_t *h)`

- **Description**: Clears the contents of the buffer.
- **Parameters**: `h` - Pointer to the buffer.

#### `void* aml_buffer_resize(aml_buffer_t *h, size_t length)`

- **Description**: Resizes the buffer, retaining the original data up to the specified length.
- **Parameters**: `h` - Pointer to the buffer, `length` - New size of the buffer.
- **Return**: Pointer to the beginning of the buffer.

#### `void* aml_buffer_shrink_by(aml_buffer_t *h, size_t length)`

- **Description**: Shrinks the buffer by the specified length. If the buffer is smaller than the specified length, it will be cleared.
- **Parameters**: `h` - Pointer to the buffer, `length` - Length to shrink by.
- **Return**: Pointer to the new memory.

### Get Contents of Buffer

### `char* aml_buffer_data(aml_buffer_t *h)`

- **Description**: Retrieves the data contained in the buffer.
- **Parameters**: `h` - Pointer to the buffer.
- **Return**: Pointer to the buffer's data.

### `size_t aml_buffer_length(aml_buffer_t *h)`

- **Description**: Gets the current length of the buffer.
- **Parameters**: `h` - Pointer to the buffer.
- **Return**: Length of the buffer.

### `char* aml_buffer_end(aml_buffer_t *h)`

- **Description**: Retrieves a pointer to the end of the buffer's contents.
- **Parameters**: `h` - Pointer to the buffer.
- **Return**: Pointer to the end of the buffer's data.

### Set Functions
These functions set the contents of the buffer.

#### `void aml_buffer_set(aml_buffer_t *h, const void *data, size_t length)`

- **Description**: Sets the buffer's contents to the specified raw data.
- **Parameters**: `h` - Pointer to the buffer, `data` - Data to set, `length` - Length of the data.

#### `void aml_buffer_sets(aml_buffer_t *h, const char *s)`

- **Description**: Sets the buffer's contents to the specified string.
- **Parameters**: `h` - Pointer to the buffer, `s` - String to set.

#### `void aml_buffer_setc(aml_buffer_t *h, char ch)`

- **Description**: Sets the buffer's contents to the specified character.
- **Parameters**: `h` - Pointer to the buffer, `ch` - Character to set.

#### `void aml_buffer_setn(aml_buffer_t *h, char ch, ssize_t n)`

- **Description**: Sets the buffer's contents to the specified character, repeated `n` times.
- **Parameters**: `h` - Pointer to the buffer, `ch` - Character to set, `n` - Number of times to repeat the character.

#### `void aml_buffer_setvf(aml_buffer_t *h, const char *fmt, va_list args)`

- **Description**: Sets the buffer's contents using a formatted string and `va_list` arguments, similar to `vprintf`.
- **Parameters**: `h` - Pointer to the buffer, `fmt` - Format string, `args` - `va_list` arguments.

#### `void aml_buffer_setf(aml_buffer_t *h, const char *fmt, ...)`

- **Description**: Sets the buffer's contents using a formatted string, similar to `printf`.
- **Parameters**: `h` - Pointer to the buffer, `fmt` - Format string, `...` - Additional format arguments.

### Append Functions
These functions are used to append various types of data to the buffer.

#### `void aml_buffer_append(aml_buffer_t *h, const void *data, size_t length)`

- **Description**: Appends raw data of specified length to the buffer.
- **Parameters**: `h` - Pointer to the buffer, `data` - Data to append, `length` - Length of the data.

#### `void aml_buffer_appends(aml_buffer_t *h, const char *s)`

- **Description**: Appends a string to the buffer.
- **Parameters**: `h` - Pointer to the buffer, `s` - String to append.

#### `void aml_buffer_appendc(aml_buffer_t *h, char ch)`

- **Description**: Appends a single character to the buffer.
- **Parameters**: `h` - Pointer to the buffer, `ch` - Character to append.

#### `void aml_buffer_appendn(aml_buffer_t *h, char ch, ssize_t n)`

- **Description**: Appends a character, repeated `n` times, to the buffer.
- **Parameters**: `h` - Pointer to the buffer, `ch` - Character to append, `n` - Number of times to repeat.

#### `void aml_buffer_appendvf(aml_buffer_t *h, const char *fmt, va_list args)`

- **Description**: Appends formatted data to the buffer using `va_list` arguments.
- **Parameters**: `h` - Pointer to the buffer, `fmt` - Format string, `args` - `va_list` arguments.

#### `void aml_buffer_appendf(aml_buffer_t *h, const char *fmt, ...)`

- **Description**: Appends formatted data to the buffer, similar to `printf`.
- **Parameters**: `h` - Pointer to the buffer, `fmt` - Format string, `...` - Additional format arguments.


### Allocation Functions
Functions to allocate memory in the buffer array.  The functions above all append or set data directly.  This allows
the space for the data to be allocated within the buffer and then for the user to modify it as needed.

#### `void* aml_buffer_append_alloc(aml_buffer_t *h, size_t length)`

- **Description**: Increases the buffer size by `length` bytes and returns a pointer to the new memory, retaining the original data.
- **Parameters**: `h` - Pointer to the buffer, `length` - Number of bytes to append.
- **Return**: Pointer to the new memory.

#### `void* aml_buffer_append_ualloc(aml_buffer_t *h, size_t length)`

- **Description**: Similar to `aml_buffer_append_alloc`, but the newly appended memory is not necessarily aligned.
- **Parameters**: `h` - Pointer to the buffer, `length` - Number of bytes to append.
- **Return**: Pointer to the new, unaligned memory.

#### `void* aml_buffer_alloc(aml_buffer_t *h, size_t length)`

- **Description**: Resizes the buffer to the specified length, but does not retain the original data.
- **Parameters**: `h` - Pointer to the buffer, `length` - New size of the buffer.
- **Return**: Pointer to the beginning of the buffer.

## Usage Example

```c
#include "a-memory-library/aml_buffer.h"

void example_usage() {
  aml_bufer_t *bh = aml_buffer_init(1024);
  aml_buffer_sets(bh, "Hello, ");
  aml_buffer_appendf(bh, "%s!", "world");
  printf( "%s\n", aml_buffer_data(bh));
  aml_buffer_clear(bh); // reuse after this
  aml_buffer_destroy(bh); 
}
```