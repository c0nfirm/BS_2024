#include "threadpool.h"

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct ThreadPool {
	/* TODO: benötigte Attribute hinzufügen */
} ThreadPool;

/* TODO: interne, globale Variablen hinzufügen */

/* TODO: interne Hilfsfunktionen hinzufügen */

int tpInit(size_t size) {
	return 0;
}

void tpRelease(void) {}

void tpAsync(Future *future) {}

void tpAwait(Future *future) {}
