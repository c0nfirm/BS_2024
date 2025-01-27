#include <stdio.h>
#include "memory.h"

#define ARR_COUNT(array) (sizeof(array)/sizeof(*array))

int main() {
	{
		mem_init();
		
		int *ptr[3];
		
		for (int i = 0; i < ARR_COUNT(ptr); ++i) {
			ptr[i] = mem_alloc(64 << i);
		}
		
		for (int i = 0; i < ARR_COUNT(ptr); i += 2) {
			mem_free(ptr[i]);
		}
		
		mem_dump(stdout);
	}
	{
		int *array;
		mem_init();
		
		array = mem_alloc(sizeof(int)*10);
		for (int i = 0; i < 10; i++)
			array[i] = i;
		
		mem_dump(stdout);
		array = mem_realloc(array, sizeof(int)*20);
		
		for (int i = 10; i < 20; i++)
			array[i] = 20-i;
		
		for (int i = 20; i --> 0;)
			printf("%i\n", array[i]);
		
		mem_dump(stdout);
		mem_free(array);
	}
}
