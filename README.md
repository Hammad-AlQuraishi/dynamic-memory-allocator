# :hammer_and_wrench: K&R Malloc Implementation in C :hammer_and_wrench:

For one of my university projects, I had to carve up a dynamic memory allocator implementation (Malloc, Free, Realloc and Calloc) based on a simplified version of sbrk() system call. I have had a growing passion to code in C, and so would like to initiate this small homework into a full time project, with further different implementations to be conjured up in the near future. Implementing them is especially helpful as that gives me RedHat and BlueHat insights on how memory allocation can be taken advantage off and how that can be remediated/prevented. 

Nonetheless, here it is.

P.S. It still needs a little bit of fixing as there are some instances which lead to an incorrect block assignment (in terms of the size) which I am working on.

## Overview

Dynamic memory allocation is a crucial aspect of programming, especially in languages like C where memory management is manual. The K&R malloc implementation provides a simple yet efficient way to allocate and deallocate memory dynamically.

## Features

- **Memory Allocation**: Provides functions for allocating memory dynamically.
- **Memory Deallocation**: Supports deallocating memory to prevent memory leaks.
- **Efficiency**: Implements memory allocation algorithm inspired by K&R for efficient memory usage.

## Usage

To use the K&R malloc implementation in your C projects, follow these steps:

1. Clone or download the repository:

    ```bash
    git clone https://github.com/Hammad-AlQuraishi/dynamic-memory-allocator.git
    ```

2. Include the `malloc.c` and `malloc.h` files in your project directory.

3. Include the `malloc.h` header file in your C source files where dynamic memory allocation is required:

    ```c
    #include "malloc.h"
    ```

4. Use the provided functions `my_malloc` and `my_free`, along with `my_realloc` & `my_calloc` for memory allocation and deallocation respectively:

    ```c
    void* ptr = my_malloc(size);
    // Use allocated memory
    my_free(ptr);
    ```

5. Compile your C program with the `malloc.c` file:

    ```bash
    gcc your_program.c malloc.c -o your_program
    ```

## Contributing

Contributions to this project are welcome. If you have any suggestions, bug reports, or feature requests, please open an issue or submit a pull request.

