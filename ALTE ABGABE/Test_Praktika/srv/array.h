/***************************************************************************//**
 * @file array.h
 * @author Dorian Weber
 * @brief Generischer Array-Typ.
 * 
 * @details
 * Diese Datei enthält die Schnittstelle eines generischen Array-Typs. Um den
 * Element-Typ des Arrays festzulegen, deklariert man eine Variable als Zeiger
 * auf den Element-Typ. Diese kann danach initialisiert und so benutzt werden,
 * als wäre sie ein Zeiger auf ein Array variabler Länge.
 * 
 * Hier ist ein Nutzungsbeispiel:
 * @code
 * int *array;
 * 
 * arrayInit(array);
 * arrayPush(array) = 1;
 * arrayPush(array) = 2;
 * arrayPush(array) = 3;
 * 
 * while (!arrayIsEmpty(array))
 * 	printf("%i\n", arrayPop(array));
 * 
 * arrayRelease(array);
 * @endcode
 * 
 * Viele der genutzten Funktionen sind in Form von Makros implementiert, die die
 * Variable, die den Zeiger auf das Array hält, aktualisieren, obwohl es so
 * aussieht, als würde sich dieser Wert niemals ändern. Das macht es riskant
 * mehr als einen Zeiger auf das Array zu halten, da jede Vergrößerung des
 * Arrays alle Zeiger auf und in das Array potentiell invalidiert. Obwohl diese
 * Fehlerquelle subtil und äußerst schwer diagnostizierbar ist - es ist eine
 * extrem einfache Abstraktion - führt diese Datenstruktur meiner Meinung nach
 * trotzdem zu einem massiven Produktivitätsgewinn; man sollte sich nur in etwa
 * im Klaren über die unterliegende Implementation sein, um genau die Untermenge
 * von Programmiertechniken auszuwählen, die korrekt funktioniert.
 ******************************************************************************/

#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

/* *** includes ************************************************************* */

#include <stddef.h>

/* *** structures *********************************************************** */

/**@brief Arrayheader.
 * 
 * Diese Struktur wird jedem Array im Speicher vorangestellt und beinhaltet
 * Informationen über Kapazität und aktuelle Auslastung des Arrays. Die
 * Arrayelemente schließen sich dieser Struktur unmittelbar an, so dass der
 * Nutzer von dieser versteckten Information nichts bemerkt.
 */
typedef struct ArrayHdr
{
	size_t len; /**<@brief Anzahl der Array-Elemente. */
	size_t cap; /**<@brief Kapazität des reservierten Speichers. */
} ArrayHdr;

/* *** interface ************************************************************ */

/**@internal
 * @brief Initialisiert und gibt einen Zeiger auf den Start des Arrays zurück.
 * @param capacity  initiale Kapazität
 * @param size      Größe der Arrayelemente
 * @return ein Zeiger auf den Start des Arrays, falls erfolgreich,\n
 *      \c NULL im Falle eines Speicherfehlers
 */
extern void* arrayInit(size_t capacity, size_t size);

/**@brief Initialisiert ein neues Array.
 * @param self  das Array
 * @return 0, falls keine Fehler bei der Initialisierung aufgetreten sind,\n
 *        -1 ansonsten
 */
#define arrayInit(self) \
	((self = arrayInit(8, sizeof((self)[0]))) == NULL ? -1 : 0)

/**@brief Gibt das Array und alle assoziierten Strukturen frei.
 * @param self  das Array
 */
extern void arrayRelease(void* self);

/**@internal
 * @brief Reserviert Platz für einen neuen Wert im Array.
 * @param self  das Array
 * @param size  Größe der Arrayelemente
 * @return der neue Zeiger auf den Start des Arrays
 */
extern void* arrayPush(void* self, size_t size);

/**@brief Legt einen Wert auf das Array.
 * @param self  das Array
 */
#define arrayPush(self) \
	(self = arrayPush(self, sizeof((self)[0])), (self)+arrayLen(self)-1)[0]

/**@brief Entfernt das oberste Element des Arrays.
 * @param self  das Array
 */
extern void arrayPop(void* self);

/**@brief Entfernt und liefert das oberste Element des Arrays.
 * @param self  das Array
 * @return das oberste Element von \p self
 */
#define arrayPop(self) \
	(arrayPop(self), (self)+arrayLen(self))[0]

/**@brief Gibt das oberste Element des Arrays zurück.
 * @param self  das Array
 * @return das oberste Element von \p self
 */
#define arrayTop(self) \
	(self)[arrayLen(self) - 1]

/**@brief Setzt die Länge des Arrays auf 0 zurück.
* @param self  das Array
*/
inline void arrayClear(void* self) {
	((ArrayHdr*) self)[-1].len = 0;
}

/**@brief Gibt zurück, ob das Array leer ist.
 * @param self  das Array
 * @return 0, falls nicht leer\n
           1, falls leer
 */
inline int arrayIsEmpty(const void* self) {
	return ((ArrayHdr*) self)[-1].len == 0;
}

/**@brief Gibt die Anzahl der Array-Elemente zurück.
 * @param self  das Array
 * @return Anzahl der Elemente des Arrays
 */
inline size_t arrayLen(const void* self) {
	return ((ArrayHdr*) self)[-1].len;
}

#endif /* ARRAY_H_INCLUDED */
