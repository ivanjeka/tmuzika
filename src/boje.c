#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <ncurses.h>

#include "boje.h"

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
	.border = 3,
	.v_border = 3,
	// NASLOV I POMOC
	.title_tmuzika = 7,	
	.title_help = 7,
	// PRIKAZ TEKST
	.volume = 7,
	.status = 7,
	.repeat = 7,
	.time = 7,
	.date = 7,
	.list = 7,
	// TMUZIKA / TRADIO
	.title_text = 7,
	// LISTA RADIO STANICA
	.play = 2,
	.song_text = 7,
	.scroll = 7,
	.playlist_mark = 2,
	// FOOTER TEKST
	.footer = 3,
	.footer_notifications = 2,
	// FILE TYPES
	.folder = 7,
	.audio = 3,
	.other = 6,
	.file_mark = 2,
	.selected_file = 2,
	// CONFIG
	.config_text = 7
};
static char tmuzika_boje_programa_putanja[PATH_MAX];
static short tmuzika_boja(short c){
	if (c < 0){
		return c;
	}
	if (c >= COLORS){
		return cista_bela;
	}
	if (c == 15){
		return cista_bela;
	}
	if (c == 8 && COLORS < 16){
		return COLOR_WHITE;
	}
	return c;
}
void tmuzika_postavi_putanju_boje(const char *folder){
	char config_folder[PATH_MAX];
	snprintf(config_folder, sizeof(config_folder), "%s", folder);
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
		"border=3\n"
		"v_border=3\n"
		"\n"
		" ----- TITLE & HELP -----\n"
		"\n"
		"title_tmuzika=7\n"
		"title_help=7\n"
		"\n"
		" ----- VIEW TEXT -----\n"
		"\n"
		"volume=7\n"
		"status=7\n"
		"time=7\n"
		"date=7\n"
		"repeat=7\n"
		"list=7\n"
		"\n"
		" ----- TMUZIKA / TRADIO -----\n"
		"\n"
		"title_text=7\n"
		"\n"
		" ----- PLAYLIST -----\n"
		"\n"
		"play=2\n"
		"song_text=7\n"
		"scroll=7\n"
		"playlist_mark=2\n"
		"\n"
		" ----- FOOTER -----\n"
		"\n"
		"footer=3\n"
		"footer_notifications=2\n"
		"\n"
		" ----- FILE TYPES -----\n"
		"\n"
		"folder=7\n"
		"audio=3\n"
		"other=6\n"
		"file_mark=2\n"
		"selected_file=2\n"
		"\n"
		" ----- CONFIGURATION -----\n"
		"\n"
		"config_text=7\n"
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
			} else if (strcmp(key, "playlist_mark") == 0){
				boje.playlist_mark = value;
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
			} else if (strcmp(key, "file_mark") == 0){
				boje.file_mark = value;
			} else if (strcmp(key, "selected_file") == 0){
				boje.selected_file = value;
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
		} else if (strcmp(key, "playlist_mark") == 0){
			boje.playlist_mark = value;
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
		} else if (strcmp(key, "file_mark") == 0){
			boje.file_mark = value;
		} else if (strcmp(key, "selected_file") == 0){
			boje.selected_file = value;
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
	// panel background (za prazne linije)
	init_pair(TMUZIKA_PANEL_TMUZIKA_BG, tmuzika_boja(cista_bela), tmuzika_boja(boje.title_panel_bg));
	init_pair(TMUZIKA_PANEL_PRIKAZ_BG, tmuzika_boja(cista_bela), tmuzika_boja(boje.view_panel_bg));
	init_pair(TMUZIKA_PANEL_FOLDER_BG, tmuzika_boja(cista_bela), tmuzika_boja(boje.folder_panel_bg));
	init_pair(TMUZIKA_PANEL_LISTA_BG, tmuzika_boja(cista_bela), tmuzika_boja(boje.playlist_panel_bg));
	init_pair(TMUZIKA_PANEL_FOOTER_BG, tmuzika_boja(cista_bela), tmuzika_boja(boje.footer_panel_bg));
	init_pair(TMUZIKA_PANEL_BORDER_BG, tmuzika_boja(cista_bela), tmuzika_boja(boje.border_panel_bg));
	init_pair(TMUZIKA_PANEL_V_BORDER_BG, tmuzika_boja(cista_bela), tmuzika_boja(boje.v_border_panel_bg));
	init_pair(TMUZIKA_PANEL_SCROLL_BG, tmuzika_boja(cista_bela), tmuzika_boja(boje.scroll_panel_bg));
	// NASLOVNI PANEL
	init_pair(TMUZIKA_NASLOV_PAIR, tmuzika_boja(boje.title_tmuzika), tmuzika_boja(boje.title_panel_bg));
	init_pair(TMUZIKA_POMOC_PAIR, tmuzika_boja(boje.title_help), tmuzika_boja(boje.title_panel_bg));
	// TMUZIKA TEXT U PANELU PRIKAZ
	init_pair(TMUZIKA_VOL_PAIR, tmuzika_boja(boje.volume), tmuzika_boja(boje.view_panel_bg));
	init_pair(TMUZIKA_STATUS_PAIR, tmuzika_boja(boje.status), tmuzika_boja(boje.view_panel_bg));
	init_pair(TMUZIKA_TIME_PAIR, tmuzika_boja(boje.time), tmuzika_boja(boje.view_panel_bg));
	init_pair(TMUZIKA_DATE_PAIR, tmuzika_boja(boje.date), tmuzika_boja(boje.view_panel_bg));
	init_pair(TMUZIKA_REPET_PAIR, tmuzika_boja(boje.repeat), tmuzika_boja(boje.view_panel_bg));
	init_pair(TMUZIKA_LIST_PAIR, tmuzika_boja(boje.list), tmuzika_boja(boje.view_panel_bg));
	// TMUZIKA FOLDER / TRADIO
	init_pair(TMUZIKA_TEKST_NASLOV_PAIR, tmuzika_boja(boje.title_text), tmuzika_boja(boje.folder_panel_bg));
	// TMUZIKA LISTA STANICA
	init_pair(TMUZIKA_PLAYLIST_SVIRA_PAIR, tmuzika_boja(boje.play), tmuzika_boja(boje.playlist_panel_bg));
	init_pair(TMUZIKA_PLAYLIST_TEKST_PAIR, tmuzika_boja(boje.song_text), tmuzika_boja(boje.playlist_panel_bg));
	init_pair(TMUZIKA_SCROLL_PAIR, tmuzika_boja(boje.scroll), tmuzika_boja(boje.scroll_panel_bg));
	init_pair(TMUZIKA_PLAYLISTMARK_PAIR, tmuzika_boja(boje.playlist_mark), -1);
	// FOOTER TEKST I OBAVESTENJE
	init_pair(TMUZIKA_PLAYLIST_FOOTER_TEKST_PAIR, tmuzika_boja(boje.footer), tmuzika_boja(boje.footer_panel_bg));
	init_pair(TMUZIKA_PLAYLIST_FOOTER_OBAVESTENJE_PAIR, tmuzika_boja(boje.footer_notifications), tmuzika_boja(boje.footer_panel_bg));
	// FILE TYPES
	init_pair(TMUZIKA_DIREKTORIJUM_PAIR, tmuzika_boja(boje.folder), -1);
	init_pair(TMUZIKA_AUDIO_PAIR, tmuzika_boja(boje.audio), -1);
	init_pair(TMUZIKA_OSTALO_PAIR, tmuzika_boja(boje.other), -1);
	init_pair(TMUZIKA_PLAYLIST_MARK_PAIR, tmuzika_boja(boje.file_mark), -1);
	init_pair(TMUZIKA_FM_SELECTEDFILE_PAIR, tmuzika_boja(boje.selected_file), -1);
	// CONFIG
	init_pair(TMUZIKA_KONFIGURACIJA_NASLOV_PAIR, tmuzika_boja(boje.config_text), -1);
	// BORDER PANEL
	init_pair(TMUZIKA_BORDER_PAIR, tmuzika_boja(boje.border), tmuzika_boja(boje.border_panel_bg));
	init_pair(TMUZIKA_V_BORDER_PAIR, tmuzika_boja(boje.v_border), tmuzika_boja(boje.v_border_panel_bg));
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
