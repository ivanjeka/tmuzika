#ifndef BOJE_H
#define BOJE_H

#include <glib.h>
#include <glib/gi18n.h>
#include <gst/gst.h>
#include <limits.h>

#define NASLOV_CRVENA_BOJA 20
#define NASLOV_ZELENA_BOJA 21
#define NASLOV_ZUTA_BOJA 22
#define NASLOV_PLAVA_BOJA 23
#define NASLOV_MAGENTA_BOJA 24
#define NASLOV_CYAN_BOJA 25
#define NASLOV_BELA_BOJA 26
#define NASLOV_SIVA_BOJA 27

#define PRIKAZ_CRVENA_BOJA 28
#define PRIKAZ_ZELENA_BOJA 29
#define PRIKAZ_ZUTA_BOJA 30
#define PRIKAZ_PLAVA_BOJA 31
#define PRIKAZ_MAGENTA_BOJA 32
#define PRIKAZ_CYAN_BOJA 33
#define PRIKAZ_BELA_BOJA 34
#define PRIKAZ_SIVA_BOJA 35

#define RADIO_CRVENA_BOJA 36
#define RADIO_ZELENA_BOJA 37
#define RADIO_ZUTA_BOJA 38
#define RADIO_PLAVA_BOJA 39
#define RADIO_MAGENTA_BOJA 40
#define RADIO_CYAN_BOJA 41
#define RADIO_BELA_BOJA 42
#define RADIO_SIVA_BOJA 43

#define LISTA_CRVENA_BOJA 44
#define LISTA_ZELENA_BOJA 45
#define LISTA_ZUTA_BOJA 46
#define LISTA_PLAVA_BOJA 47
#define LISTA_MAGENTA_BOJA 48
#define LISTA_CYAN_BOJA 49
#define LISTA_BELA_BOJA 50
#define LISTA_SIVA_BOJA 51

#define TMUZIKA_CRVENA_BOJA 52
#define TMUZIKA_ZELENA_BOJA 53
#define TMUZIKA_ZUTA_BOJA 54
#define TMUZIKA_PLAVA_BOJA 55
#define TMUZIKA_MAGENTA_BOJA 56
#define TMUZIKA_CYAN_BOJA 57
#define TMUZIKA_BELA_BOJA 58
#define TMUZIKA_SIVA_BOJA 59

#define FOOTER_CRVENA_BOJA 60
#define FOOTER_ZELENA_BOJA 61
#define FOOTER_ZUTA_BOJA 62
#define FOOTER_PLAVA_BOJA 63
#define FOOTER_MAGENTA_BOJA 64
#define FOOTER_CYAN_BOJA 65
#define FOOTER_BELA_BOJA 66
#define FOOTER_SIVA_BOJA 67

// PANELI
#define TMUZIKA_PANEL_V_BORDER_BG 68
#define TMUZIKA_PANEL_BORDER_BG 69
#define TMUZIKA_PANEL 70
#define TMUZIKA_PANEL_PRIKAZ_BG 71
#define TMUZIKA_PANEL_FOLDER_BG 72
#define TMUZIKA_PANEL_LISTA_BG 73
#define TMUZIKA_PANEL_TMUZIKA_BG 74
#define TMUZIKA_PANEL_FOOTER_BG 75

// NASLOV
#define TMUZIKA_NASLOV_PAIR 76
#define TMUZIKA_POMOC_PAIR 77

// TRADIO TEXT U PANELU PRIKAZ
#define TMUZIKA_VOL_PAIR    78
#define TMUZIKA_STATUS_PAIR 79
#define TMUZIKA_TIME_PAIR   80
#define TMUZIKA_REPET_PAIR   81
#define TMUZIKA_LIST_PAIR 82

// TRADIO LISTA STANICA
#define TMUZIKA_PLAYLIST_MARK_PAIR 83
#define TMUZIKA_PLAYLIST_SVIRA_PAIR 84
#define TMUZIKA_PLAYLIST_TEKST_PAIR 85

// FOOTER TEXT I OBAVESTENJE
#define TMUZIKA_PLAYLIST_FOOTER_TEKST_PAIR 86
#define TMUZIKA_PLAYLIST_FOOTER_OBAVESTENJE_PAIR 87

// TRADIO TEKST
#define TMUZIKA_TEKST_NASLOV_PAIR 88
#define TMUZIKA_DIREKTORIJUM_PAIR 89
#define TMUZIKA_SCROLL_PAIR 90
#define TMUZIKA_KONFIGURACIJA_NASLOV_PAIR 91
#define TMUZIKA_OSTALO_PAIR 92
#define TMUZIKA_AUDIO_PAIR 93
#define TMUZIKA_BORDER_PAIR 94
#define TMUZIKA_DATE_PAIR   95
#define TMUZIKA_V_BORDER_PAIR 96
#define TMUZIKA_PANEL_SCROLL_BG 97

#define BORDER_CRVENA_BOJA 100
#define BORDER_ZELENA_BOJA 101
#define BORDER_ZUTA_BOJA 102
#define BORDER_PLAVA_BOJA 103
#define BORDER_MAGENTA_BOJA 104
#define BORDER_CYAN_BOJA 105
#define BORDER_BELA_BOJA 106
#define BORDER_SIVA_BOJA 107
// VERTICAL BORDER
#define VBORDER_CRVENA_BOJA 108
#define VBORDER_ZELENA_BOJA 109
#define VBORDER_ZUTA_BOJA 110
#define VBORDER_PLAVA_BOJA 111
#define VBORDER_MAGENTA_BOJA 112
#define VBORDER_CYAN_BOJA 113
#define VBORDER_BELA_BOJA 114
#define VBORDER_SIVA_BOJA 115
// SCROLL
#define SCROLL_CRVENA_BOJA 116
#define SCROLL_ZELENA_BOJA 117
#define SCROLL_ZUTA_BOJA 118
#define SCROLL_PLAVA_BOJA 119
#define SCROLL_MAGENTA_BOJA 120
#define SCROLL_CYAN_BOJA 121
#define SCROLL_BELA_BOJA 122
#define SCROLL_SIVA_BOJA 123

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
	short mark;
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
	// CONFIG
	short config_text;
} BojeTmuzikaPrograma;

extern BojeTmuzikaPrograma boje;
void tmuzika_postavi_putanju_boje(const char *folder);
void tmuzika_proveri_boje_fajl(void);
void tmuzika_boje_programa_konfiguracija1(void);
void tmuzika_reset_boje_prompt();

void tmuzika_ucitaj_boje(void);
void tmuzika_init_boje(void);

#endif