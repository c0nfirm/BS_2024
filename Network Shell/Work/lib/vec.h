/***************************************************************************//**
 * @file vec.h
 * @author Dorian Weber
 * @brief Generischer Vektor-Typ.
 * 
 * @details
 * Diese Datei enthält die Schnittstelle eines generischen Vektor-Typs. Um den
 * Element-Typ des Vektors festzulegen, deklariert man eine Variable als Zeiger
 * auf den Element-Typ. Diese kann danach initialisiert und so benutzt werden,
 * als wäre sie ein Zeiger auf einen Vektor variabler Länge.
 * 
 * Hier ist ein Nutzungsbeispiel:
 * @code
 * int *vec;
 * 
 * vecInit(vec);
 * vecPush(vec) = 1;
 * vecPush(vec) = 2;
 * vecPush(vec) = 3;
 * 
 * while (!vecIsEmpty(vec))
 * 	printf("%i\n", vecPop(vec));
 * 
 * vecRelease(vec);
 * @endcode
 * 
 * Viele der genutzten Funktionen sind in Form von Makros implementiert, die die
 * Variable, die den Zeiger auf den Vektor hält, aktualisieren, obwohl es so
 * aussieht, als würde sich dieser Wert niemals ändern. Das macht es riskant
 * mehr als einen Zeiger auf den Vektor zu halten, da jede Vergrößerung des
 * Vektors alle Zeiger auf und in den Vektor potentiell invalidiert. Obwohl diese
 * Fehlerquelle subtil und äußerst schwer diagnostizierbar ist - es ist eine
 * extrem einfache Abstraktion - führt diese Datenstruktur meiner Meinung nach
 * trotzdem zu einem massiven Produktivitätsgewinn; man sollte sich nur in etwa
 * im Klaren über die unterliegende Implementation sein, um genau die Untermenge
 * von Programmiertechniken auszuwählen, die korrekt funktioniert.
 ******************************************************************************/

#ifndef VEC_H_INCLUDED
#define VEC_H_INCLUDED

/* *** includes ************************************************************* */

#include <stddef.h>

/* *** structures *********************************************************** */

/**@brief Vektorheader.
 * 
 * Diese Struktur wird jedem Vektor im Speicher vorangestellt und beinhaltet
 * Informationen über Kapazität und aktuelle Auslastung des Vektors. Die
 * Vektorelemente schließen sich dieser Struktur unmittelbar an, so dass der
 * Nutzer von dieser versteckten Information nichts bemerkt.
 */
typedef struct VecHdr {
	size_t len; /**<@brief Anzahl der Elemente. */
	size_t cap; /**<@brief Kapazität des reservierten Speichers. */
} VecHdr;

/* *** interface ************************************************************ */

/**@internal
 * @brief Initialisiert und gibt einen Zeiger auf den Start des Vektors zurück.
 * @param capacity  initiale Kapazität
 * @param size      Größe der Vektorelemente
 * @return ein Zeiger auf den Start des Vektors, falls erfolgreich,\n
 *      \c NULL im Falle eines Speicherfehlers
 */
extern void* vecInit(size_t capacity, size_t size);

/**@brief Initialisiert einen neuen Vektor.
 * @param self  der Vektor
 * @return 0, falls keine Fehler bei der Initialisierung aufgetreten sind,\n
 *        -1 ansonsten
 */
#define vecInit(self) \
	((self = vecInit(8, sizeof((self)[0]))) == NULL ? -1 : 0)

/**@brief Gibt den Vektor und alle assoziierten Strukturen frei.
 * @param self  der Vektor
 */
extern void vecRelease(void* self);

/**@internal
 * @brief Reserviert Platz für einen neuen Wert im Vektor.
 * @param self  der Vektor
 * @param size  Größe der Vektorelemente
 * @return der neue Zeiger auf den Start des Vektors
 */
extern void* vecPush(void* self, size_t size);

/**@brief Legt einen Wert auf den Vektor.
 * @param self  der Vektor
 */
#define vecPush(self) \
	(self = vecPush(self, sizeof((self)[0])), (self)+vecLen(self)-1)[0]

/**@brief Entfernt das oberste Element des Vektors.
 * @param self  der Vektor
 */
extern void vecPop(void* self);

/**@brief Entfernt und liefert das oberste Element des Vektors.
 * @param self  der Vektor
 * @return das oberste Element von \p self
 */
#define vecPop(self) \
	(vecPop(self), (self)+vecLen(self))[0]

/**@brief Gibt das oberste Element des Vektors zurück.
 * @param self  der Vektor
 * @return das oberste Element von \p self
 */
#define vecTop(self) \
	(self)[vecLen(self) - 1]

/**@brief Setzt die Länge des Vektors auf 0 zurück.
* @param self  der Vektor
*/
inline void vecClear(void* self) {
	((VecHdr*) self)[-1].len = 0;
}

/**@brief Gibt zurück, ob der Vektor leer ist.
 * @param self  der Vektor
 * @return 0, falls nicht leer\n
           1, falls leer
 */
inline int vecIsEmpty(const void* self) {
	return ((VecHdr*) self)[-1].len == 0;
}

/**@brief Gibt die Anzahl der Vektor-Elemente zurück.
 * @param self  der Vektor
 * @return Anzahl der Elemente des Vektors
 */
inline size_t vecLen(const void* self) {
	return ((VecHdr*) self)[-1].len;
}

#endif /* VEC_H_INCLUDED */
