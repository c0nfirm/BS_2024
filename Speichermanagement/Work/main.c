#include <stdio.h>
#include "memory.h"

int main() {
	int *array;
	mem_init();
	
	array = mem_alloc(sizeof(int)*10);
	if (array == NULL)
		perror("mem_alloc()"), exit(-1);
	
	for (int i = 0; i < 10; i++)
		array[i] = i;
	
	array = mem_realloc(array, sizeof(int)*20);
	if (array == NULL)
		perror("mem_realloc()"), exit(-1);
	
	for (int i = 10; i < 20; i++)
		array[i] = 20-i;
	
	for (int i = 20; i --> 0;)
		printf("%i\n", array[i]);
	
	mem_free(array);
}
