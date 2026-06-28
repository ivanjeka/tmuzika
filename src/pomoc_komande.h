#ifndef POMOC_KOMANDE_H
#define POMOC_KOMANDE_H

#include <ncurses.h>

void pomoc_komande(WINDOW *win);

const char *tmuzika_key_to_string(int key);

extern void provera_zavrsetka_pesme(void);
extern void footer_pomoc(WINDOW *win);

#endif