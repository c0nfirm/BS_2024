#include "threadpool.h"

#include <stdio.h>
#include <stdlib.h>

static TASK(long, fib, long);

long fib(long n) {
	if (n <= 1)
		return n;
	
	fib_fut *a = fibAsync((fib_fut[]) { fibFuture(n - 1) });
	fib_fut *b = fibAsync((fib_fut[]) { fibFuture(n - 2) });
	
	return fibAwait(a) + fibAwait(b);
}

int main() {
	if (tpInit(8) != 0)
		perror("Thread Pool initialization failed"), exit(-1);
	atexit(&tpRelease);
	
	for (long i = 0; i <= 20; ++i)
		printf("fib(%2li) = %li\n", i, fib(i));
	
	return 0;
}
