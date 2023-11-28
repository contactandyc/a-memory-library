# AML Buffer Management ([aml_buffer.h](../include/a-memory-library/aml_buffer.h))

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
