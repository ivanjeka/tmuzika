#ifndef BOJE_H
#define BOJE_H

#include <glib.h>
#include <glib/gi18n.h>
#include <gst/gst.h>
#include <limits.h>

#define SIVA_BOJA 1
#define CISTA_BELA 2

// PANELI
#define TMUZIKA_PANEL_V_BORDER_BG 3
#define TMUZIKA_PANEL_BORDER_BG 4
#define TMUZIKA_PANEL_PRIKAZ_BG 5
#define TMUZIKA_PANEL_FOLDER_BG 6
#define TMUZIKA_PANEL_LISTA_BG 7
#define TMUZIKA_PANEL_TMUZIKA_BG 8
#define TMUZIKA_PANEL_FOOTER_BG 9

// NASLOV
#define TMUZIKA_NASLOV_PAIR 10
#define TMUZIKA_POMOC_PAIR 11

// TRADIO TEXT U PANELU PRIKAZ
#define TMUZIKA_VOL_PAIR 12
#define TMUZIKA_STATUS_PAIR 13
#define TMUZIKA_TIME_PAIR 14
#define TMUZIKA_REPET_PAIR 15
#define TMUZIKA_LIST_PAIR 16

// TRADIO LISTA STANICA
#define TMUZIKA_PLAYLIST_MARK_PAIR 17
#define TMUZIKA_PLAYLIST_SVIRA_PAIR 18
#define TMUZIKA_PLAYLIST_TEKST_PAIR 19

// FOOTER TEXT I OBAVESTENJE
#define TMUZIKA_PLAYLIST_FOOTER_TEKST_PAIR 20
#define TMUZIKA_PLAYLIST_FOOTER_OBAVESTENJE_PAIR 21

// OSTALE BOJE
#define TMUZIKA_TEKST_NASLOV_PAIR 22
#define TMUZIKA_DIREKTORIJUM_PAIR 23
#define TMUZIKA_SCROLL_PAIR 24
#define TMUZIKA_KONFIGURACIJA_NASLOV_PAIR 25
#define TMUZIKA_OSTALO_PAIR 26
#define TMUZIKA_AUDIO_PAIR 27
#define TMUZIKA_BORDER_PAIR 28
#define TMUZIKA_DATE_PAIR 29
#define TMUZIKA_V_BORDER_PAIR 30
#define TMUZIKA_PANEL_SCROLL_BG 31
#define TMUZIKA_PLAYLISTMARK_PAIR 32
#define TMUZIKA_FM_SELECTEDFILE_PAIR 33

typedef struct {
	// PANELI
	short title_panel_bg;
	short view_panel_bg;
	short folder_panel_bg;
	short playlist_panel_bg;
	short footer_panel_bg;
	short border_panel_bg;
	short border;
	short v_border_panel_bg;
	short v_border;
	short scroll_panel_bg;
	// NASLOV I POMOC
	short title_tmuzika;	
	short title_help;
	// PRIKAZ TEKST
	short volume;
	short status;
	short repeat;
	short time;
	short date;
	short list;
	// LISTA RADIO STANICA
	short playlist_mark;
	short play;
	short song_text;
	short scroll;
	// FOOTER TEKST
	short footer;
	short footer_notifications;
	// FOLDER TEXT
	short title_text;
	// FILE TYPES
	short folder;
	short audio;
	short other;
	short file_mark;
	short selected_file;
	// CONFIG
	short config_text;
} BojeTmuzikaPrograma;

extern int cpu_ciscenje;
extern short cista_bela;
extern short SIVA_FG;
extern BojeTmuzikaPrograma boje;
void tmuzika_postavi_putanju_boje(const char *folder);
void tmuzika_proveri_boje_fajl(void);
void tmuzika_boje_programa_konfiguracija1(void);
void tmuzika_reset_boje_prompt();

void tmuzika_ucitaj_boje(void);
void tmuzika_init_boje(void);

#endif