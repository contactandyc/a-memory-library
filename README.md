# A memory library

## Installation

### Clone the library and change to the directory

```bash
git clone https://github.com/contactandyc/a-memory-library.git
cd a-memory-library
```

### Build and install library

```bash
mkdir -p build
cd build
cmake ..
make
make install
```

# Overview of A Memory Library (AML)

AML is a memory management library in C, offering a range of functions for efficient handling of memory allocation, buffer management, and memory pooling.  The library is designed to work with binary and string data and can help track memory bugs when using the debug build of the library.  The pool and buffer can help to dramatically reduce memory fragmentation and system calls.

## AML Memory Management Functions ([aml_alloc.h](docs/aml_alloc.md))

### Core Allocation Functions
- **aml_malloc**: For allocating uninitialized memory.
- **aml_calloc**: For allocating memory for an array and initializing it to zero.
- **aml_zalloc**: For allocating zero-initialized memory.
- **aml_realloc**: For reallocating memory blocks to a new size.

### Memory Freeing
- **aml_free**: To free allocated memory space.

### String and Memory Duplication
- **aml_strdup**: For duplicating strings.
- **aml_strdupf**: For creating formatted strings.
- **aml_strdupvf**: For creating formatted strings using `va_list`.
- **aml_dup**: For duplicating raw memory.

### Array of Strings Duplication
- **aml_strdupa**: For duplicating NULL-terminated arrays of strings as a single allocation.
- **aml_strdupan**: Similar to `aml_strdupa`, but with a specified length.
- **aml_strdupa2**: Duplicates only the pointers in a NULL-terminated array of strings.

## AML Pool Memory Management ([aml_pool.h](docs/aml_pool.md))

### Pool Initialization and Management
- **aml_pool_init**: Initializes a memory pool.
- **aml_pool_pool_init**: Creates a sub-pool from an existing pool.
- **aml_pool_clear**: Clears the memory pool.
- **aml_pool_destroy**: Destroys the memory pool.

### Pool Statistics
- **aml_pool_size**: Returns the total bytes allocated from the pool.
- **aml_pool_used**: Provides the total bytes allocated by the pool itself, including overhead.

### Memory Allocation in Pool
- **aml_pool_alloc**: Allocates uninitialized memory from the pool.
- **aml_pool_calloc**: Allocates and zero-initializes memory for an array.
- **aml_pool_zalloc**: Allocates zero-initialized memory.
- **aml_pool_min_max_alloc**: Allocates memory with a size between two specified values.
- **aml_pool_ualloc**: Allocates uninitialized memory without alignment.

### String Manipulation in Pool
- **aml_pool_strdup**: Duplicates a string using the pool.
- **aml_pool_strdupf**: Creates a formatted string using the pool.
- **aml_pool_strdupvf**: Similar to `aml_pool_strdupf`, but with `va_list`.
- **aml_pool_strndup**: Duplicates a string with a maximum length.
- **aml_pool_dup**: Duplicates a data block.
- **aml_pool_udup**: Duplicates a data block without alignment.

### String Splitting Functions
- **aml_pool_split**: Splits a string into an array using a delimiter.
- **aml_pool_splitf**: Splits a formatted string.
- **aml_pool_split2**: Splits a string excluding empty strings.
- **aml_pool_split2f**: Splits a formatted string excluding empty strings.

### Array Duplication Functions
- **aml_pool_strdupa**: Duplicates an array of strings.
- **aml_pool_strdupan**: Duplicates a specified number of strings in an array.
- **aml_pool_strdupa2**: Duplicates the structure of an array of strings.

## AML Buffer Management ([aml_buffer.h](docs/aml_buffer.md))

### Buffer Initialization and Destruction
- **aml_buffer_init**: Initializes a buffer with a specified size.
- **aml_buffer_pool_init**: Initializes a buffer using a memory pool.
- **aml_buffer_destroy**: Destroys the buffer.

### Buffer Clearing and Resizing
- **aml_buffer_clear**: Clears the buffer.
- **aml_buffer_resize**: Resizes the buffer while retaining original data.
- **aml_buffer_shrink_by**: Shrinks the buffer by a specified length.

### Buffer Content Management
- **aml_buffer_data**: Retrieves buffer data.
- **aml_buffer_length**: Gets buffer length.
- **aml_buffer_end**: Retrieves a pointer to the end of buffer's content.

#### Set Functions
- **aml_buffer_set**: Sets buffer's contents to specified raw data (`data`, `length`).
- **aml_buffer_sets**: Sets buffer's contents to a specified string (`s`).
- **aml_buffer_setc**: Sets buffer's contents to a single character (`ch`).
- **aml_buffer_setn**: Sets buffer's contents to a character repeated `n` times (`ch`, `n`).
- **aml_buffer_setvf**: Sets buffer's contents using formatted string with `va_list` arguments (`fmt`, `args`).
- **aml_buffer_setf**: Sets buffer's contents using a formatted string (`fmt`, ...).

#### Append Functions
- **aml_buffer_append**: Appends specified length of raw data to buffer (`data`, `length`).
- **aml_buffer_appends**: Appends a string to the buffer (`s`).
- **aml_buffer_appendc**: Appends a single character to the buffer (`ch`).
- **aml_buffer_appendn**: Appends a character, repeated `n` times, to the buffer (`ch`, `n`).
- **aml_buffer_appendvf**: Appends formatted data to buffer using `va_list` arguments (`fmt`, `args`).
- **aml_buffer_appendf**: Appends formatted data to the buffer (`fmt`, ...).

### Memory Allocation in Buffer
- **aml_buffer_append_alloc**: Increases buffer size, retaining original data.
- **aml_buffer_append_ualloc**: Similar to `append_alloc` but with unaligned memory.
- **aml_buffer_alloc**: Resizes the buffer without retaining original data.

