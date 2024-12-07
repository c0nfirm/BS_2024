/***************************************************************************//**
 * @file vec.c
 * @author Dorian Weber
 * @brief Implementation des generalisierten Vektors.
 ******************************************************************************/

#include "vec.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* ********************************************************* public functions */

/* (die runden Klammern um einige Funktionsnamen sind notwendig, da Makros
 * gleichen Namens existieren und der Präprozessor diese expandieren würde) */

void* (vecInit)(size_t capacity, size_t size) {
	VecHdr *hdr = malloc(sizeof(*hdr) + size*capacity);
	
	if (hdr == NULL)
		return NULL;
	
	hdr->len = 0;
	hdr->cap = capacity;
	
	return hdr + 1;
}

void vecRelease(void* self) {
	free(((VecHdr*) self) - 1);
}

void* (vecPush)(void* self, size_t size) {
	VecHdr *hdr = ((VecHdr*) self) - 1;
	
	if (hdr->len == hdr->cap) {
		hdr->cap *= 2;
		hdr = realloc(hdr, sizeof(*hdr) + size*hdr->cap);
		
		if (hdr == NULL) {
			fputs("program ran out of heap memory\n", stderr);
			exit(-1);
		}
	}
	
	++hdr->len;
	return hdr + 1;
}

void (vecPop)(void* self) {
	VecHdr *hdr = ((VecHdr*) self) - 1;
	assert(hdr->len > 0);
	--hdr->len;
}

/* Symbol für die Inline-Funktionen erzeugen und aus diesem Modul exportieren */
extern void vecClear(void* self);
extern int vecIsEmpty(const void* self);
extern size_t vecLen(const void* self);
