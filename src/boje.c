#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <ncurses.h>

#include "boje.h"

extern int cpu_ciscenje;

BojeTmuzikaPrograma boje = {
	// POZADINA PANELA
	.title_panel_bg = -1,
	.view_panel_bg = -1,
	.folder_panel_bg = -1,
	.playlist_panel_bg = -1,
	.footer_panel_bg = -1,
	.border_panel_bg = -1,
	.v_border_panel_bg = -1,
	.scroll_panel_bg = -1,
	// BORDER
	.border = 8,
	.v_border = 8,
	// NASLOV I POMOC
	.title_tmuzika = 15,	
	.title_help = 15,
	// PRIKAZ TEKST
	.volume = 15,
	.status = 15,
	.repeat = 15,
	.time = 15,
	.date = 15,
	.list = 15,
	// TMUZIKA / TRADIO
	.title_text = 15,
	// LISTA RADIO STANICA
	.play = 2,
	.song_text = 15,
	.scroll = 15,
	// FOOTER TEKST
	.footer = 8,
	.footer_notifications = 2,
	// FILE TYPES
	.folder = 15,
	.audio = 3,
	.other = 4,
	.mark = 1,
	// CONFIG
	.config_text = 15
};
static char tmuzika_boje_programa_putanja[PATH_MAX];

void tmuzika_postavi_putanju_boje(const char *folder){
	char config_folder[PATH_MAX];
	snprintf(config_folder, sizeof(config_folder), "%s/config", folder);
	if (mkdir(config_folder, 0755) == -1 && errno != EEXIST){
		perror("mkdir config_folder");
		return;
	}
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-truncation"
	snprintf(tmuzika_boje_programa_putanja, sizeof(tmuzika_boje_programa_putanja), "%s/tmuzika_boje", config_folder);
	#pragma GCC diagnostic pop
}
static void tmuzika_napravi_default_boje(){
	FILE *f = fopen(tmuzika_boje_programa_putanja, "w");
	if (!f){
		return;
	}
	fprintf(f,
		"# TMUZIKA color configuration\n"
		"# Use color numbers\n"
		"# black=0, red=1, green=2, yellow=3, blue=4, magenta=5, cyan=6, white=7, gray=8\n"
		"# light_red=9, light_green=10, light_yellow=11, light_blue=12, light_magenta=13\n" 
		"# light_cyan=14, light_white=15\n"
		"\n"
		"\n"
		" ----- BACKGROUND PANELS (-1 = use terminal default color) -----\n"
		"\n"
		"title_panel_bg=-1\n"
		"view_panel_bg=-1\n"
		"folder_panel_bg=-1\n"
		"playlist_panel_bg=-1\n"
		"footer_panel_bg=-1\n"
		"border_panel_bg=-1\n"
		"v_border_panel_bg=-1\n"
		"scroll_panel_bg=-1\n"
		"\n"
		" ----- BORDERS -----\n"
		"\n"
		"border=8\n"
		"v_border=8\n"
		"\n"
		" ----- TITLE & HELP -----\n"
		"\n"
		"title_tmuzika=15\n"
		"title_help=15\n"
		"\n"
		" ----- VIEW TEXT -----\n"
		"\n"
		"volume=15\n"
		"status=15\n"
		"time=15\n"
		"date=15\n"
		"repeat=15\n"
		"list=15\n"
		"\n"
		" ----- TMUZIKA / TRADIO -----\n"
		"\n"
		"title_text=15\n"
		"\n"
		" ----- PLAYLIST -----\n"
		"\n"
		"play=2\n"
		"song_text=15\n"
		"scroll=15\n"
		"\n"
		" ----- FOOTER -----\n"
		"\n"
		"footer=8\n"
		"footer_notifications=2\n"
		"\n"
		" ----- FILE TYPES -----\n"
		"\n"
		"folder=15\n"
		"audio=3\n"
		"other=4\n"
		"mark=1\n"
		"\n"
		" ----- CONFIGURATION -----\n"
		"\n"
		"config_text=15\n"
	);
	fclose(f);
}
void tmuzika_proveri_boje_fajl(){
	struct stat st;
	if (stat(tmuzika_boje_programa_putanja, &st) != 0){
		tmuzika_napravi_default_boje();
	}
}
void tmuzika_boje_programa_konfiguracija1(){
	FILE *f = fopen(tmuzika_boje_programa_putanja, "r");
	if (!f){
		return;
	}
	char line[128];
	while (fgets(line, sizeof(line), f)){
		if (line[0] == '#' || strlen(line) < 3){
			continue;
		}
		char key[64];
		int value;
		if (sscanf(line, "%63[^=]=%d", key, &value) == 2){
			// PANELI
			if (strcmp(key, "title_panel_bg") == 0){
				boje.title_panel_bg = value;
			} else if (strcmp(key, "view_panel_bg") == 0){
				boje.view_panel_bg = value;
			} else if (strcmp(key, "folder_panel_bg") == 0){
				boje.folder_panel_bg = value;
			} else if (strcmp(key, "playlist_panel_bg") == 0){
				boje.playlist_panel_bg = value;
			} else if (strcmp(key, "footer_panel_bg") == 0){
				boje.footer_panel_bg = value;
			} else if (strcmp(key, "border_panel_bg") == 0){
				boje.border_panel_bg = value;
			} else if (strcmp(key, "v_border_panel_bg") == 0){
				boje.v_border_panel_bg = value;
			} else if (strcmp(key, "scroll_panel_bg") == 0){
				boje.scroll_panel_bg = value;
			} else if (strcmp(key, "border") == 0){
				boje.border = value;
			} else if (strcmp(key, "v_border") == 0){
				boje.v_border = value;
			// NASLOV I POMOC
			} else if (strcmp(key, "title_tmuzika") == 0){
				boje.title_tmuzika = value;
			} else if (strcmp(key, "title_help") == 0){
				boje.title_help = value;
			// PRIKAZ TEKST
			} else if (strcmp(key, "volume") == 0){
				boje.volume = value;
			} else if (strcmp(key, "status") == 0){
				boje.status = value;
			} else if (strcmp(key, "time") == 0){
				boje.time = value;
			} else if (strcmp(key, "date") == 0){
				boje.date = value;
			} else if (strcmp(key, "repeat") == 0){
				boje.repeat = value;
			} else if (strcmp(key, "list") == 0){
				boje.list = value;
			// TMUZIKA / TRADIO
			}  else if (strcmp(key, "title_text") == 0){
				boje.title_text = value;
			// LISTA RADIO STANICA
			} else if (strcmp(key, "play") == 0){
				boje.play = value;
			} else if (strcmp(key, "song_text") == 0){
				boje.song_text = value;
			} else if (strcmp(key, "scroll") == 0){
				boje.scroll = value;
			// FOOTER TEKST
			} else if (strcmp(key, "footer") == 0){
				boje.footer = value;
			} else if (strcmp(key, "footer_notifications") == 0){
				boje.footer_notifications = value;
			// FILE TYPE
			} else if (strcmp(key, "folder") == 0){
				boje.folder = value;
			} else if (strcmp(key, "audio") == 0){
				boje.audio = value;
			} else if (strcmp(key, "other") == 0){
				boje.other = value;
			} else if (strcmp(key, "mark") == 0){
				boje.mark = value;
			// CONFIGURATION
			} else if (strcmp(key, "config_text") == 0){
				boje.config_text = value;
			}
		}
	}
	fclose(f);
}
void tmuzika_ucitaj_boje(void) {
	struct stat st;
	if (stat(tmuzika_boje_programa_putanja, &st) != 0) {
		tmuzika_napravi_default_boje();
	}
	FILE *f = fopen(tmuzika_boje_programa_putanja, "r");
	if (!f){
		return;
	}
	char line[128];
	while (fgets(line, sizeof(line), f)) {
		if (line[0] == '#' || strlen(line) < 3) continue;
		char key[64];
		int value;
		if (sscanf(line, "%63[^=]=%d", key, &value) != 2){
			continue;
		}
		// PANELI
		if (strcmp(key, "title_panel_bg") == 0){
			boje.title_panel_bg = value;
		} else if (strcmp(key, "view_panel_bg") == 0){
			boje.view_panel_bg = value;
		} else if (strcmp(key, "folder_panel_bg") == 0){
			boje.folder_panel_bg = value;
		} else if (strcmp(key, "playlist_panel_bg") == 0){
			boje.playlist_panel_bg = value;
		} else if (strcmp(key, "footer_panel_bg") == 0){
			boje.footer_panel_bg = value;
		} else if (strcmp(key, "border_panel_bg") == 0){
			boje.border_panel_bg = value;
		} else if (strcmp(key, "v_border_panel_bg") == 0){
			boje.v_border_panel_bg = value;
		} else if (strcmp(key, "scroll_panel_bg") == 0){
			boje.scroll_panel_bg = value;
		} else if (strcmp(key, "border") == 0){
			boje.border = value;
		} else if (strcmp(key, "v_border") == 0){
			boje.v_border = value;
		// NASLOV I POMOC
		} else if (strcmp(key, "title_tmuzika") == 0){
			boje.title_tmuzika = value;
		} else if (strcmp(key, "title_help") == 0){
			boje.title_help = value;
		// PRIKAZ TEKST
		} else if (strcmp(key, "volume") == 0){
			boje.volume = value;
		} else if (strcmp(key, "status") == 0){
			boje.status = value;
		} else if (strcmp(key, "time") == 0){
			boje.time = value;
		} else if (strcmp(key, "date") == 0){
			boje.date = value;
		} else if (strcmp(key, "repeat") == 0){
			boje.repeat = value;
		} else if (strcmp(key, "list") == 0){
			boje.list = value;
		// TMUZIKA / TRADIO
		}  else if (strcmp(key, "title_text") == 0){
			boje.title_text = value;
		// LISTA RADIO STANICA
		} else if (strcmp(key, "play") == 0){
			boje.play = value;
		} else if (strcmp(key, "song_text") == 0){
			boje.song_text = value;
		} else if (strcmp(key, "scroll") == 0){
			boje.scroll = value;
		// FOOTER TEKST
		} else if (strcmp(key, "footer") == 0){
			boje.footer = value;
		} else if (strcmp(key, "footer_notifications") == 0){
			boje.footer_notifications = value;
		// FILE TYPE
		} else if (strcmp(key, "folder") == 0){
			boje.folder = value;
		} else if (strcmp(key, "audio") == 0){
			boje.audio = value;
		} else if (strcmp(key, "other") == 0){
			boje.other = value;
		} else if (strcmp(key, "mark") == 0){
			boje.mark = value;
		// CONFIGURATION
		} else if (strcmp(key, "config_text") == 0){
			boje.config_text = value;
		}
	}
	fclose(f);
}
void tmuzika_init_boje(void) {
	start_color();
	use_default_colors();

	short SIVA_BOJA;
	if (COLORS >= 16){
		SIVA_BOJA = 8;
	} else{
		SIVA_BOJA = COLOR_WHITE;
	}
	// panel background (za prazne linije)
	init_pair(TMUZIKA_PANEL_TMUZIKA_BG, 15, boje.title_panel_bg);
	init_pair(TMUZIKA_PANEL_PRIKAZ_BG, 15, boje.view_panel_bg);
	init_pair(TMUZIKA_PANEL_FOLDER_BG, 15, boje.folder_panel_bg);
	init_pair(TMUZIKA_PANEL_LISTA_BG, 15, boje.playlist_panel_bg);
	init_pair(TMUZIKA_PANEL_FOOTER_BG, 15, boje.footer_panel_bg);
	init_pair(TMUZIKA_PANEL_BORDER_BG, 15, boje.border_panel_bg);
	init_pair(TMUZIKA_PANEL_V_BORDER_BG, 15, boje.v_border_panel_bg);
	init_pair(TMUZIKA_PANEL_SCROLL_BG, 15, boje.scroll_panel_bg);
	// PANEL 1
	init_pair(NASLOV_CRVENA_BOJA, COLOR_RED, boje.title_panel_bg);
	init_pair(NASLOV_ZELENA_BOJA, COLOR_GREEN, boje.title_panel_bg);
	init_pair(NASLOV_ZUTA_BOJA, COLOR_YELLOW, boje.title_panel_bg);
	init_pair(NASLOV_PLAVA_BOJA, COLOR_BLUE, boje.title_panel_bg);
	init_pair(NASLOV_MAGENTA_BOJA, COLOR_MAGENTA, boje.title_panel_bg);
	init_pair(NASLOV_CYAN_BOJA, COLOR_CYAN, boje.title_panel_bg);
	init_pair(NASLOV_BELA_BOJA, 15, boje.title_panel_bg);
	init_pair(NASLOV_SIVA_BOJA, SIVA_BOJA, boje.title_panel_bg);
	// PANEL 2
	init_pair(PRIKAZ_CRVENA_BOJA, COLOR_RED, boje.view_panel_bg);
	init_pair(PRIKAZ_ZELENA_BOJA, COLOR_GREEN, boje.view_panel_bg);
	init_pair(PRIKAZ_ZUTA_BOJA, COLOR_YELLOW, boje.view_panel_bg);
	init_pair(PRIKAZ_PLAVA_BOJA, COLOR_BLUE, boje.view_panel_bg);
	init_pair(PRIKAZ_MAGENTA_BOJA, COLOR_MAGENTA, boje.view_panel_bg);
	init_pair(PRIKAZ_CYAN_BOJA, COLOR_CYAN, boje.view_panel_bg);
	init_pair(PRIKAZ_BELA_BOJA, 15, boje.view_panel_bg);
	init_pair(PRIKAZ_SIVA_BOJA, SIVA_BOJA, boje.view_panel_bg);
	// PANEL 3
	init_pair(RADIO_CRVENA_BOJA, COLOR_RED, boje.folder_panel_bg);
	init_pair(RADIO_ZELENA_BOJA, COLOR_GREEN, boje.folder_panel_bg);
	init_pair(RADIO_ZUTA_BOJA, COLOR_YELLOW, boje.folder_panel_bg);
	init_pair(RADIO_PLAVA_BOJA, COLOR_BLUE, boje.folder_panel_bg);
	init_pair(RADIO_MAGENTA_BOJA, COLOR_MAGENTA, boje.folder_panel_bg);
	init_pair(RADIO_CYAN_BOJA, COLOR_CYAN, boje.folder_panel_bg);
	init_pair(RADIO_BELA_BOJA, 15, boje.folder_panel_bg);
	init_pair(RADIO_SIVA_BOJA, SIVA_BOJA, boje.folder_panel_bg);
	// PANEL 4
	init_pair(LISTA_CRVENA_BOJA, COLOR_RED, boje.playlist_panel_bg);
	init_pair(LISTA_ZELENA_BOJA, COLOR_GREEN, boje.playlist_panel_bg);
	init_pair(LISTA_ZUTA_BOJA, COLOR_YELLOW, boje.playlist_panel_bg);
	init_pair(LISTA_PLAVA_BOJA, COLOR_BLUE, boje.playlist_panel_bg);
	init_pair(LISTA_MAGENTA_BOJA, COLOR_MAGENTA, boje.playlist_panel_bg);
	init_pair(LISTA_CYAN_BOJA, COLOR_CYAN, boje.playlist_panel_bg);
	init_pair(LISTA_BELA_BOJA, 15, boje.playlist_panel_bg);
	init_pair(LISTA_SIVA_BOJA, SIVA_BOJA, boje.playlist_panel_bg);
	// PANEL 5
	init_pair(FOOTER_CRVENA_BOJA, COLOR_RED, boje.footer_panel_bg);
	init_pair(FOOTER_ZELENA_BOJA, COLOR_GREEN, boje.footer_panel_bg);
	init_pair(FOOTER_ZUTA_BOJA, COLOR_YELLOW, boje.footer_panel_bg);
	init_pair(FOOTER_PLAVA_BOJA, COLOR_BLUE, boje.footer_panel_bg);
	init_pair(FOOTER_MAGENTA_BOJA, COLOR_MAGENTA, boje.footer_panel_bg);
	init_pair(FOOTER_CYAN_BOJA, COLOR_CYAN, boje.footer_panel_bg);
	init_pair(FOOTER_BELA_BOJA, 15, boje.footer_panel_bg);
	init_pair(FOOTER_SIVA_BOJA, SIVA_BOJA, boje.footer_panel_bg);
	// BORDER
	init_pair(BORDER_CRVENA_BOJA, COLOR_RED, boje.border_panel_bg);
	init_pair(BORDER_ZELENA_BOJA, COLOR_GREEN, boje.border_panel_bg);
	init_pair(BORDER_ZUTA_BOJA, COLOR_YELLOW, boje.border_panel_bg);
	init_pair(BORDER_PLAVA_BOJA, COLOR_BLUE, boje.border_panel_bg);
	init_pair(BORDER_MAGENTA_BOJA, COLOR_MAGENTA, boje.border_panel_bg);
	init_pair(BORDER_CYAN_BOJA, COLOR_CYAN, boje.border_panel_bg);
	init_pair(BORDER_BELA_BOJA, 15, boje.border_panel_bg);
	init_pair(BORDER_SIVA_BOJA, SIVA_BOJA, boje.border_panel_bg);
	// VERTICAL BORDER
	init_pair(VBORDER_CRVENA_BOJA, COLOR_RED, boje.v_border_panel_bg);
	init_pair(VBORDER_ZELENA_BOJA, COLOR_GREEN, boje.v_border_panel_bg);
	init_pair(VBORDER_ZUTA_BOJA, COLOR_YELLOW, boje.v_border_panel_bg);
	init_pair(VBORDER_PLAVA_BOJA, COLOR_BLUE, boje.v_border_panel_bg);
	init_pair(VBORDER_MAGENTA_BOJA, COLOR_MAGENTA, boje.v_border_panel_bg);
	init_pair(VBORDER_CYAN_BOJA, COLOR_CYAN, boje.v_border_panel_bg);
	init_pair(VBORDER_BELA_BOJA, 15, boje.v_border_panel_bg);
	init_pair(VBORDER_SIVA_BOJA, SIVA_BOJA, boje.v_border_panel_bg);
	// SCROLL
	init_pair(SCROLL_CRVENA_BOJA, COLOR_RED, boje.scroll_panel_bg);
	init_pair(SCROLL_ZELENA_BOJA, COLOR_GREEN, boje.scroll_panel_bg);
	init_pair(SCROLL_ZUTA_BOJA, COLOR_YELLOW, boje.scroll_panel_bg);
	init_pair(SCROLL_PLAVA_BOJA, COLOR_BLUE, boje.scroll_panel_bg);
	init_pair(SCROLL_MAGENTA_BOJA, COLOR_MAGENTA, boje.scroll_panel_bg);
	init_pair(SCROLL_CYAN_BOJA, COLOR_CYAN, boje.scroll_panel_bg);
	init_pair(SCROLL_BELA_BOJA, 15, boje.scroll_panel_bg);
	init_pair(SCROLL_SIVA_BOJA, SIVA_BOJA, boje.scroll_panel_bg);
	// NASLOVNI PANEL
	init_pair(TMUZIKA_NASLOV_PAIR, boje.title_tmuzika, boje.title_panel_bg);
	init_pair(TMUZIKA_POMOC_PAIR, boje.title_help, boje.title_panel_bg);
	// TMUZIKA TEXT U PANELU PRIKAZ
	init_pair(TMUZIKA_VOL_PAIR, boje.volume, boje.view_panel_bg);
	init_pair(TMUZIKA_STATUS_PAIR, boje.status, boje.view_panel_bg);
	init_pair(TMUZIKA_TIME_PAIR, boje.time, boje.view_panel_bg);
	init_pair(TMUZIKA_DATE_PAIR, boje.date, boje.view_panel_bg);
	init_pair(TMUZIKA_REPET_PAIR, boje.repeat, boje.view_panel_bg);
	init_pair(TMUZIKA_LIST_PAIR, boje.list, boje.view_panel_bg);
	// TMUZIKA FOLDER / TRADIO
	init_pair(TMUZIKA_TEKST_NASLOV_PAIR, boje.title_text, boje.folder_panel_bg);
	// TMUZIKA LISTA STANICA
	init_pair(TMUZIKA_PLAYLIST_SVIRA_PAIR, boje.play, boje.playlist_panel_bg);
	init_pair(TMUZIKA_PLAYLIST_TEKST_PAIR, boje.song_text, boje.playlist_panel_bg);
	init_pair(TMUZIKA_SCROLL_PAIR, boje.scroll, boje.scroll_panel_bg);
	// FOOTER TEKST I OBAVESTENJE
	init_pair(TMUZIKA_PLAYLIST_FOOTER_TEKST_PAIR, boje.footer, boje.footer_panel_bg);
	init_pair(TMUZIKA_PLAYLIST_FOOTER_OBAVESTENJE_PAIR, boje.footer_notifications, boje.footer_panel_bg);
	// FILE TYPES
	init_pair(TMUZIKA_DIREKTORIJUM_PAIR, boje.folder, -1);
	init_pair(TMUZIKA_AUDIO_PAIR, boje.audio, -1);
	init_pair(TMUZIKA_OSTALO_PAIR, boje.other, -1);
	init_pair(TMUZIKA_PLAYLIST_MARK_PAIR, boje.mark, -1);
	// CONFIG
	init_pair(TMUZIKA_KONFIGURACIJA_NASLOV_PAIR, boje.config_text, -1);
	// BORDER PANEL
	init_pair(TMUZIKA_BORDER_PAIR, boje.border, boje.border_panel_bg);
	init_pair(TMUZIKA_V_BORDER_PAIR, boje.v_border, boje.v_border_panel_bg);
}
void tmuzika_reset_boje_prompt(){
	werase(stdscr);
	refresh();
	wattron(stdscr, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
	box(stdscr, 0, 0);
	wattroff(stdscr, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
	mvprintw(1, 1, _("Are you sure you want to reset color configuration? (y/N)"));
	refresh();
	wtimeout(stdscr, -1);
	int c = getch();
	wtimeout(stdscr, 200);
	if (c == 'y' || c == 'Y') {
		remove(tmuzika_boje_programa_putanja);
		tmuzika_napravi_default_boje();
		tmuzika_boje_programa_konfiguracija1();
		tmuzika_ucitaj_boje();
		tmuzika_init_boje();
		clear();
		refresh();
	} else {
		cpu_ciscenje = 1;
	}
}