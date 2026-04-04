#ifndef TMUZIKA_H
#define TMUZIKA_H

#include <glib.h>
#include <glib/gtypes.h>   // za gdouble

/* --- Struktura za listu pesama --- */
typedef struct {
	GPtrArray *stavke;          // lista pesama
	GPtrArray *backup_stavke;   // backup lista
	int trenutna;               // indeks trenutno selektovane pesme
	int svira;                  // indeks pesme koja trenutno svira
	gdouble volume;             // jačina zvuka (0.0 - 1.0)
} Lista_pesama;

#endif
