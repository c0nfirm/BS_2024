#ifndef THREADPOOL_H_INCLUDED
#define THREADPOOL_H_INCLUDED

#include <stddef.h>

/**@brief Funktionszeiger auf eine asynchron auszuführende Funktion.
 * 
 * Der Parameter kann zur Übergabe von Argumenten und den Rückgabewerten
 * der Funktion genutzt werden.
 */
typedef void (*ThreadTask_f)(void*);

/**@brief Handle zum zukünftigen Rückgabewert eines asynchronen Funktionsrufes.
 */
typedef struct Future {
	ThreadTask_f fn;   ///<@brief Zeiger auf die auszuführende Funktion.
	/* TODO: benötigte Attribute hinzufügen */
} Future;

/**@brief Initialisiert den globalen Thread-Pool.
 * 
 * Diese Funktion legt \p size viele Worker-Threads an und initialisiert den
 * restlichen Zustand des Thread-Pools. Danach werden keine weiteren Threads
 * mehr durch den Thread-Pool angelegt.
 * 
 * @param size  Anzahl der Worker-Threads
 */
extern int tpInit(size_t size);

/**@brief Beendet alle Worker-Threads im Thread-Pool kontrolliert und gibt
 * sämtliche Systemressourcen des Thread-Pools frei.
 * 
 * Bereits gestartete Futures können abgebrochen oder fertiggestellt werden.
 */
extern void tpRelease(void);

/**@brief Startet eine nutzerdefinierte Future.
 * 
 * Die Funktion initialisiert die übergebene Future und fügt sie zur Liste
 * der abzuarbeitenden Tasks hinzu. Die Abarbeitungsreihenfolge ist
 * unspezifiziert und die Abarbeitung startet sofort (eager).
 * 
 * @param future  partiell-initialisiertes Future-Objekt
 * @note Der Rufer ist dafür verantwortlich, dass der Speicher nicht vor
 * der Auswertung des asynchronen Funktionsrufes freigegeben wird.
 */
extern void tpAsync(Future *future);

/**@brief Erwartet die Abarbeitung einer Future.
 * 
 * Nachdem diese Funktion zurückgekommen ist, ist der asynchrone Funktionsruf
 * garantiert abgeschlossen und der Zugriff auf die Future Data-Race-frei.
 * Die Funktion gibt auch alle Systemressourcen des Thread-Pools frei.
 * 
 * @param future  asynchron gestartetes Future-Objekt
 * @note Der Rufer kann danach den Speicher für die Future wieder freigeben.
 */
extern void tpAwait(Future *future);

/**@brief Erzeugt Code für eine spezialisierte Schnittstelle einer Funktion.
 * 
 * Die direkte Nutzung der minimalistischen Schnittstelle des Thread-Pools ist
 * unkomfortabel und fehleranfällig, daher ist hier ein Makro definiert,
 * mithilfe dessen eine spezifische Schnittstelle für asynchrone Funktionen
 * erzeugt werden kann.
 * 
 * Das Makro muss im Filescope expandiert werden und erzeugt folgende Elemente:
 *   1. eine spezifische Datenstruktur für Argumente und Rückgabewerte der
 *      konkreten Funktion mit dem Namen `func_fut`
 *   2. eine sogenannte thunk-Funktion mit dem Namen `funcThunk`, welche die
 *      Funktionsargumente aus dem generischen Argument entpackt, die konkrete
 *      Funktion ausführt und dann den Rückgabewert zurückschreibt
 *   3. eine asynchrone Ruf-Funktion mit dem Namen `funcAsync`, welche ein
 *      Future-Objekt initialisiert und zurückgibt
 *   4. eine synchrone Warte-Funktion mit dem Namen `funcAwait`, welche auf die
 *      Abarbeitung eines zuvor asynchron gerufenen Future-Objektes wartet und
 *      das Ergebnis zurückgibt
 * 
 * Zusätzlich dazu wird die konkrete Funktion anfangs deklariert.
 * Benutzung ist wie folgt möglich:
 * @code
 * 	// deklariert die Schnittstelle einer Funktion ´long fib(long)´
 * 	TASK(long, fib, long);
 * @endcode
 * 
 * @param TYPE  Rückgabetyp der Funktion
 * @param NAME  Name der Funktion
 * @param ARG   Parametertyp der Funktion
 */
#define TASK(TYPE, NAME, ARG) \
	TYPE NAME(ARG); \
	\
	typedef struct { \
		Future fut;  \
		ARG    arg;  \
		TYPE   res;  \
	} NAME ## _fut;  \
	\
	static void NAME ## Thunk(void *args) { \
		NAME ## _fut *data = args;          \
		data->res = NAME(data->arg);        \
	} \
	static inline NAME ## _fut NAME ## Future(ARG arg) { \
		return (NAME ## _fut) {                          \
			.fut = { .fn = &NAME ## Thunk },             \
			.arg = arg                                   \
		};                                               \
	} \
	static inline NAME ## _fut* NAME ## Async(NAME ## _fut *future) { \
		return tpAsync(&future->fut), future;                         \
	} \
	static inline TYPE NAME ## Await(NAME ## _fut *future) { \
		return tpAwait(&future->fut), future->res;           \
	}

#endif
