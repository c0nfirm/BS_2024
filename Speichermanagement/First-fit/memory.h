
#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

/**@brief Initializes the memory allocator to a consistent state.
 */
extern void mem_init();

/**@brief Allocates a block of memory, returning a pointer.
 */
extern void* mem_alloc(size_t size);

/**@brief Increases or decreases the amount of allocated memory.
 */
extern void* mem_realloc(void *oldptr, size_t new_size);

/**@brief Frees an allocated block of memory.
 */
extern void mem_free(void *ptr);

/**@brief Prints free pages for every order.
 */
extern void mem_dump(FILE *file);

#endif
