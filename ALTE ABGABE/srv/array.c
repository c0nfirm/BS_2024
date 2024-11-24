/***************************************************************************//**
 * @file array.c
 * @author Dorian Weber
 * @brief Implementation des generalisierten Arrays.
 ******************************************************************************/

#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* ********************************************************* public functions */

/* (die runden Klammern um einige Funktionsnamen sind notwendig, da Makros
 * gleichen Namens existieren und der Präprozessor diese expandieren würde) */

void* (arrayInit)(size_t capacity, size_t size) {
	ArrayHdr *hdr = malloc(sizeof(*hdr) + size*capacity);
	
	if (hdr == NULL)
		return NULL;
	
	hdr->len = 0;
	hdr->cap = capacity;
	
	return hdr + 1;
}

void arrayRelease(void* self) {
	free(((ArrayHdr*) self) - 1);
}

void* (arrayPush)(void* self, size_t size) {
	ArrayHdr *hdr = ((ArrayHdr*) self) - 1;
	
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

void (arrayPop)(void* self) {
	ArrayHdr *hdr = ((ArrayHdr*) self) - 1;
	assert(hdr->len > 0);
	--hdr->len;
}

/* Symbol für die Inline-Funktionen erzeugen und aus diesem Modul exportieren */
extern void arrayClear(void* self);
extern int arrayIsEmpty(const void* self);
extern size_t arrayLen(const void* self);
