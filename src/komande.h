#ifndef KOMANDE_H
#define KOMANDE_H

#include <glib.h>
#include <glib/gi18n.h>
#include <gst/gst.h>
#include <limits.h>
#include <ncurses.h>

typedef struct {
	// GLOBAL
	short tmuzika_scroll_left;
	short tmuzika_scroll_right;
	short tmuzika_reset_key;
	short tmuzika_reset_color;
	// TMUZIKA
	short tmuzika_play;
	short tmuzika_pause;
	short tmuzika_stop;
	short tmuzika_next;
	short tmuzika_preview;
	short tmuzika_volume_up;
	short tmuzika_volume_down;
	short tmuzika_seek_f;
	short tmuzika_seek_b;
	short tmuzika_repete_song;
	short tmuzika_repete_all;
	short tmuzika_shutlle;
	short tmuzika_search;
	short tmuzika_radio;
	short tmuzika_file_manager;
	short tmuzika_save_m3u;
	short tmuzika_load_m3u;
	short tmuzika_current_song;
	short tmuzika_delete_song;
	short tmuzika_delete_all;
	short tmuzika_first_song;
	short tmuzika_last_song;
	short tmuzika_exit;
	// TRADIO
	short tradio_play;
	short tradio_pause;
	short tradio_stop;
	short tradio_next;
	short tradio_preview;
	short tradio_volume_up;
	short tradio_volume_down;
	short tradio_add_state_f;
	short tradio_search;
	short tradio_file_manager;
	short tradio_add;
	short tradio_save_m3u;
	short tradio_load_m3u;
	short tradio_current_song;
	short tradio_delete_song;
	short tradio_delete_all;
	short tradio_first_song;
	short tradio_last_song;
	short tradio_exit;
} KomandePrograma;

extern KomandePrograma komande;
void tmuzika_postavi_putanju_komande(const char *folder);
void proveri_komande_fajl(void);
void komande_programa_konfiguracija1(void);
void tmuzika_reset_komande_prompt();

void ucitaj_komande(void);

#endif