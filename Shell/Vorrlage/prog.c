#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static unsigned int hash(const void *key, size_t size) {
	const char *ptr = key;
	unsigned int hval;
	
	for (hval = 0x811c9dc5u; size --> 0; ++ptr) {
		hval ^= *ptr;
		hval *= 0x1000193u;
	}
	
	return hval;
}

static inline int roll(int sides) {
	return rand() / (RAND_MAX + 1.0) * sides;
}

int main(int argc, const char* argv[]) {
	int rc, delay;
	time_t now = time(0);
	
	/* initialize the random number generator */
	srand(hash(&now, sizeof(now)));
	
	/* determine exit code and time delay */
	switch (argc) {
	default:
		delay = atoi(argv[1]);
		rc = atoi(argv[2]);
		break;
		
	case 2:
		delay = atoi(argv[1]);
		rc = roll(2) - 1;
		break;
		
	case 1:
		delay = roll(10);
		rc = roll(2) - 1;
		break;
	}
	
	printf("Delay: %ds, Exit Code: %d\n", delay, rc);
	sleep(delay);
	
	return rc;
}
