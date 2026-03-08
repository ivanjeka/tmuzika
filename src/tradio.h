#ifndef TRADIO_H
#define TRADIO_H

#include <glib.h>   // za GPtrArray
#include <glib/gtypes.h> // za gdouble

/* --- Struktura za pojedinačnu radio stanicu --- */
typedef struct {
    char *name;   // naziv stanice
    char *url;    // URL stream-a
} RadioStanica;

/* --- Struktura za listu radio stanica i status --- */
typedef struct {
    GPtrArray *stanice;          // lista svih stanica
    GPtrArray *backup_stanice;   // backup lista (npr. za undo)
    int trenutna;                // indeks trenutno selektovane stanice
    int svira;                   // indeks stanice koja trenutno svira
    int online;                  // 1 = streaming, 0 = zaustavljeno
    int top_index;               // gornji indeks u prikazu (ncurses scroll)
    int visible_rows;            // koliko redova prikazano
    gdouble volume;              // jačina zvuka (0.0 - 1.0)
} RadioLista;

#endif /* RADIO_H */
