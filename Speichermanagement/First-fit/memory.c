#include <errno.h>
#include <string.h>

#include "memory.h"

/**@brief Order of the largest possible memory block. */
#define ORDER_MAX 10

/**@brief Size of the smallest possible memory block. */
#define PAGE_SIZE (1u << 6)

/**@brief Size of available memory. */
#define HEAP_SIZE (PAGE_SIZE << ORDER_MAX)

typedef struct block_s {
	size_t size;
	struct block_s *next;
} block_t;

/**@brief Heap memory. */
static char heap[HEAP_SIZE];
static block_t *free_list;

void mem_init() {
	block_t *node = (block_t*) heap;
	node->next = NULL;
	node->size = HEAP_SIZE;
	free_list = node;
}

void* mem_alloc(size_t size) {
	block_t *block, **pred;
	
	// reserve space for the preamble
	size += sizeof(size_t);
	
	// ensure that the space is at least a page
	if (size < PAGE_SIZE)
		size = PAGE_SIZE;
	
	// round up to fulfill alignment restrictions
	size = (size + 7) &~ 7;
	
	// find a suitably sized block
	pred = &free_list, block = free_list;
	while (1) {
		// is the block big enough?
		if (block->size >= size)
			break;
		
		// remember the next-pointer and select the next block
		pred = &block->next, block = block->next;
		
		// no more free blocks left?
		if (block == NULL)
			goto fail;
	}
	
	// see if we have to split the block
	if (block->size - size >= PAGE_SIZE) {
		block_t *newBlock = (block_t*) (((char*) block) + size);
		newBlock->size = block->size - size;
		newBlock->next = block->next;
		block->size = size;
		block->next = newBlock;
	}
	
	// remove the block from the list
	*pred = block->next;
	
	// return it
	return ((size_t*) block) + 1;
	
fail:
	errno = ENOMEM;
	return NULL;
}

void* mem_realloc(void *oldptr, size_t new_size) {
	void *newBlock = mem_alloc(new_size);
	
	if (newBlock == NULL)
		goto fail;
	
	memcpy(newBlock, oldptr, ((size_t*) oldptr)[-1]);
	mem_free(oldptr);
	
	return newBlock;
	
fail:
	errno = ENOMEM;
	return NULL;
}

void mem_free(void *ptr) {
	block_t *block = (block_t*) (((char*) ptr) - sizeof(size_t));
	block->next = free_list;
	free_list = block;
}

void mem_dump(FILE *file) {
	size_t total_mem = 0, cont_mem = 0;
	
	fputs("heap: {\n", file);
	fputs("\tblocks: [\n", file);
	for (block_t* block = free_list; block != NULL; block = block->next) {
		fprintf(file, "\t\t{ off: %lu, len: %lu },\n", (char*) block - heap, block->size);
		total_mem += block->size;
		
		if (block->size > cont_mem)
			cont_mem = block->size;
	}
	
	fputs("\t],\n", file);
	fprintf(
		file,
		"\tfree: %lu,\n"
		"\tcont: %lu\n",
		total_mem,
		cont_mem
	);
	fputs("}\n", file);
}
