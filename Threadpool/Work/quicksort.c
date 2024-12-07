#include "threadpool.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static size_t partition(int list[], size_t len);

/* TODO: quicksort-Implementation parallelisieren */

static void quicksort(int list[], size_t len) {
	if (len <= 1) return;
	
	size_t mid = partition(list, len);
	quicksort(list, mid);
	quicksort(list + mid + 1, len - mid - 1);
}

size_t partition(int v[], size_t len) {
	size_t pivot = len - 1;
	size_t l = 0, r = len - 2;
	int t;
	
	while (1) {
		while (l < r && v[l] <  v[pivot]) ++l;
		while (l < r && v[r] >= v[pivot]) --r;
		
		if (l >= r) break;
		
		t = v[l];
		v[l] = v[r];
		v[r] = t;
	}
	
	if (v[l] >= v[pivot]) {
		t = v[l];
		v[l] = v[pivot];
		v[pivot] = t;
		return l;
	}
	
	return pivot;
}

static void list_randomize(int list[], size_t len) {
	for (int *p = list, *e = p + len; p < e; ++p)
		*p = ((((double) rand()) / RAND_MAX) - 0.5) * len;
}

static void list_print(int list[], size_t len) {
	const char *sep = "";
	
	fputs("(", stdout);
	for (int *p = list, *e = p + len; p < e; ++p, sep = ", ")
		printf("%s%i", sep, *p);
	fputs(")\n", stdout);
}

static int list_is_sorted(int list[], size_t len) {
	for (int *p = list, *e = p + len - 1; p < e; ++p) {
		if (p[0] > p[1])
			return 0;
	}
	
	return 1;
}

static struct timespec time_diff(struct timespec t1, struct timespec t2) {
	struct timespec result = {
		.tv_sec  = t2.tv_sec - t1.tv_sec,
		.tv_nsec = t2.tv_nsec - t1.tv_nsec
	};
	
	if (t2.tv_nsec - t1.tv_nsec < 0) {
		result.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		result.tv_nsec = 1000000000l + t2.tv_nsec - t1.tv_nsec;
	}
	
	return result;
}

int main(int argc, const char *argv[]) {
	if (tpInit(8) != 0)
		perror("Thread Pool initialization failed"), exit(-1);
	atexit(&tpRelease);
	
	/* read the length of the list from the command line */
	size_t list_len = 30000000;
	if (argc >= 2) {
		long arg = atol(argv[1]);
		if (arg > 0)
			list_len = arg;
	}
	
	/* generate a list with random integers */
	int *list = malloc(sizeof(int) * list_len);
	list_randomize(list, list_len);
	printf("Len: %lu\n", list_len);
	
	/* print the unsorted list if it's not too long */
	printf("---- Unsortiert: %i ----\n", list_is_sorted(list, list_len));
	
	if (list_len <= 100)
		list_print(list, list_len);
	
	/* sort it and print the sorted list if it's not too long */
	struct timespec start, end, dt;
	clock_gettime(CLOCK_REALTIME, &start);
	quicksort(list, list_len);
	clock_gettime(CLOCK_REALTIME, &end);
	dt = time_diff(start, end);
	printf("---- Sortiert:   %i ----\n", list_is_sorted(list, list_len));
	
	if (list_len <= 100)
		list_print(list, list_len);
	
	printf("Dauer: %lis %lins\n", dt.tv_sec, dt.tv_nsec);
	return 0;
}
