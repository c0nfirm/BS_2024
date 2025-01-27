#include <errno.h>

#include "memory.h"
#include "bitset.h"

/**@brief Order of the largest possible memory block. */
#define ORDER_MAX 10

/**@brief Size of the smallest possible memory block. */
#define PAGE_SIZE 64

/**@brief Size of available memory. */
#define HEAP_SIZE (PAGE_SIZE << ORDER_MAX)

/**@brief Heap memory. */
static char heap[HEAP_SIZE];


void mem_init() {
	/* TODO: initialize the data structures for the allocator */
}

void* mem_alloc(size_t size) {
	/* TODO: locate or create a block of appropriate order */
fail:
	errno = ENOMEM;
	return NULL;
}

void* mem_realloc(void *oldptr, size_t new_size) {
	/* TODO: increase the size of an existing block through merging
	 *   OR  allocate a new one with identical contents */
fail:
	errno = ENOMEM;
	return NULL;
}

void mem_free(void *ptr) {
	/* TODO: mark a block as unused */
}

void mem_dump(FILE *file) {
	/* TODO: print the current state of the allocator */
}
