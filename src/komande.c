#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "komande.h"
#include "boje.h"

extern int cpu_ciscenje;

KomandePrograma komande = {
	// GLOBAL
	.tmuzika_scroll_left = '[',
	.tmuzika_scroll_right = ']',
	.tmuzika_reset_key = 'h',
	.tmuzika_reset_color = 'b',
	// TMUZIKA
	.tmuzika_play = KEY_ENTER,
	.tmuzika_pause = ' ',
	.tmuzika_stop = 'z',
	.tmuzika_next = '>',
	.tmuzika_preview = '<',
	.tmuzika_volume_up = '+',
	.tmuzika_volume_down = '-',
	.tmuzika_seek_f = KEY_RIGHT,
	.tmuzika_seek_b = KEY_LEFT,
	.tmuzika_repete_song = 'p',
	.tmuzika_repete_all = 'l',
	.tmuzika_shutlle = 'e',
	.tmuzika_search = 's',
	.tmuzika_radio = 'r',
	.tmuzika_file_manager = 'm',
	.tmuzika_save_m3u = 'q',
	.tmuzika_load_m3u = 'u',
	.tmuzika_current_song = 'v',
	.tmuzika_delete_song = 'x',
	.tmuzika_delete_all = KEY_DC,
	.tmuzika_first_song = KEY_HOME,
	.tmuzika_last_song = KEY_END,
	.tmuzika_exit = 'k',
	// TRADIO
	.tradio_play = KEY_ENTER,
	.tradio_pause = ' ',
	.tradio_stop = 'z',
	.tradio_next = '>',
	.tradio_preview = '<',
	.tradio_volume_up = '+',
	.tradio_volume_down = '-',
	.tradio_add_state_f = 10,
	.tradio_search = 's',
	.tradio_file_manager = 'm',
	.tradio_add = 'd',
	.tradio_save_m3u = 'q',
	.tradio_load_m3u = 'u',
	.tradio_current_song = 'v',
	.tradio_delete_song = 'x',
	.tradio_delete_all = KEY_DC,
	.tradio_first_song = KEY_HOME,
	.tradio_last_song = KEY_END,
	.tradio_exit = 27
};
static char komande_programa_putanja[PATH_MAX];

void tmuzika_postavi_putanju_komande(const char *folder){
	char config_folder[PATH_MAX];
	snprintf(config_folder, sizeof(config_folder), "%s/config", folder);
	if (mkdir(config_folder, 0755) == -1 && errno != EEXIST){
		perror("mkdir config_folder");
		return;
	}
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-truncation"
	snprintf(komande_programa_putanja, sizeof(komande_programa_putanja), "%s/tmuzika_komande", config_folder);
	#pragma GCC diagnostic pop
}
static int tmuzika_parse_taster(const char *s) {
	if (!s || !*s){
		return -1;
	}
	if (s[0] == '\'' && s[2] == '\'') {
		return (int)s[1];
	}
	if (strcmp(s, "KEY_ENTER") == 0){
		return KEY_ENTER;
	}
	if (strcmp(s, "KEY_RIGHT") == 0){
		return KEY_RIGHT;
	}
	if (strcmp(s, "KEY_LEFT") == 0){
		return KEY_LEFT;
	}
	if (strcmp(s, "KEY_DC") == 0){
		return KEY_DC;
	}
	if (strcmp(s, "KEY_HOME") == 0){
		return KEY_HOME;
	}
	if (strcmp(s, "KEY_END") == 0){
		return KEY_END;
	}
	if (isdigit((unsigned char)s[0])){
		return atoi(s);
	}
    	return -1;
}
static void napravi_default_komande(){
	FILE *f = fopen(komande_programa_putanja, "w");
	if (!f){
		return;
	}
	fprintf(f,
		"# ---------- COMMANDS ----------\n"
		"#\n"
		"#\n"
		"# ----- TMUZIKA -----\n"
		"\n"
		"tmuzika_stop='z'\n"
		"tmuzika_repete_song='p'\n"
		"tmuzika_repete_all='l'\n"
		"tmuzika_shutlle='e'\n"
		"tmuzika_radio='r'\n"
		"tmuzika_save_m3u='q'\n"
		"tmuzika_load_m3u='u'\n"
		"tmuzika_current_song='v'\n"
		"tmuzika_delete_song='x'\n"
		"\n"
		"# ----- TRADI -----\n"
		"\n"
		"tradio_stop='z'\n"
		"tradio_add_state_f=10\n"
		"tradio_add='d'\n"
		"tradio_save_m3u='q'\n"
		"tradio_load_m3u='u'\n"
		"tradio_current_song='v'\n"
		"tradio_delete_song='x'\n"
		"\n"
	);
	fclose(f);
}
void proveri_komande_fajl(){
	struct stat st;
	if (stat(komande_programa_putanja, &st) != 0){
		napravi_default_komande();
	}
}
void komande_programa_konfiguracija1(){
	FILE *f = fopen(komande_programa_putanja, "r");
	if (!f){
		return;
	}
	char line[128];
	while (fgets(line, sizeof(line), f)){
		if (line[0] == '#' || strlen(line) < 3){
			continue;
		}
		char key[64];
		char value[64];
		if (sscanf(line, "%63[^=]=%63s", key, value) != 2){
			continue;
		}
		int kod = tmuzika_parse_taster(value);
		if (kod < 0){
			continue;
		}
		// TMUZIKA
		if (strcmp(key, "tmuzika_play") == 0){
			komande.tmuzika_play = kod;
		} else if (strcmp(key, "tmuzika_pause") == 0){
			komande.tmuzika_pause = kod;
		} else if (strcmp(key, "tmuzika_stop") == 0){
			komande.tmuzika_stop = kod;
		} else if (strcmp(key, "tmuzika_next") == 0){
			komande.tmuzika_next = kod;
		} else if (strcmp(key, "tmuzika_preview") == 0){
			komande.tmuzika_preview = kod;
		} else if (strcmp(key, "tmuzika_volume_up") == 0){
			komande.tmuzika_volume_up = kod;
		} else if (strcmp(key, "tmuzika_volume_down") == 0){
			komande.tmuzika_volume_down = kod;
		} else if (strcmp(key, "tmuzika_seek_f") == 0){
			komande.tmuzika_seek_f = kod;
		} else if (strcmp(key, "tmuzika_seek_b") == 0){
			komande.tmuzika_seek_b = kod;
		} else if (strcmp(key, "tmuzika_repete_song") == 0){
			komande.tmuzika_repete_song = kod;
		} else if (strcmp(key, "tmuzika_repete_all") == 0){
			komande.tmuzika_repete_all = kod;
		} else if (strcmp(key, "tmuzika_shutlle") == 0){
			komande.tmuzika_shutlle = kod;
		} else if (strcmp(key, "tmuzika_search") == 0){
			komande.tmuzika_search = kod;
		} else if (strcmp(key, "tmuzika_radio") == 0){
			komande.tmuzika_radio = kod;
		} else if (strcmp(key, "tmuzika_file_manager") == 0){
			komande.tmuzika_file_manager = kod;
		} else if (strcmp(key, "tmuzika_save_m3u") == 0){
			komande.tmuzika_save_m3u = kod;
		} else if (strcmp(key, "tmuzika_load_m3u") == 0){
			komande.tmuzika_load_m3u = kod;
		} else if (strcmp(key, "tmuzika_current_song") == 0){
			komande.tmuzika_current_song = kod;
		} else if (strcmp(key, "tmuzika_delete_song") == 0){
			komande.tmuzika_delete_song = kod;
		} else if (strcmp(key, "tmuzika_delete_all") == 0){
			komande.tmuzika_delete_all = kod;
		} else if (strcmp(key, "tmuzika_first_song") == 0){
			komande.tmuzika_first_song = kod;
		} else if (strcmp(key, "tmuzika_last_song") == 0){
			komande.tmuzika_last_song = kod;
		} else if (strcmp(key, "tmuzika_exit") == 0){
			komande.tmuzika_exit = kod;
		// TRADIO
		} else if (strcmp(key, "tradio_play") == 0){
			komande.tradio_play = kod;
		} else if (strcmp(key, "tradio_pause") == 0){
			komande.tradio_pause = kod;
		} else if (strcmp(key, "tradio_stop") == 0){
			komande.tradio_stop = kod;
		} else if (strcmp(key, "tradio_next") == 0){
			komande.tradio_next = kod;
		} else if (strcmp(key, "tradio_preview") == 0){
			komande.tradio_preview = kod;
		} else if (strcmp(key, "tradio_volume_up") == 0){
			komande.tradio_volume_up = kod;
		} else if (strcmp(key, "tradio_volume_down") == 0){
			komande.tradio_volume_down = kod;
		} else if (strcmp(key, "tradio_add_state_f") == 0){
			komande.tradio_add_state_f = kod;
		} else if (strcmp(key, "tradio_search") == 0){
			komande.tradio_search = kod;
		} else if (strcmp(key, "tradio_file_manager") == 0){
			komande.tradio_file_manager = kod;
		} else if (strcmp(key, "tradio_add") == 0){
			komande.tradio_load_m3u = kod;
		} else if (strcmp(key, "tradio_save_m3u") == 0){
			komande.tradio_save_m3u = kod;
		} else if (strcmp(key, "tradio_load_m3u") == 0){
			komande.tradio_load_m3u = kod;
		} else if (strcmp(key, "tradio_current_song") == 0){
			komande.tradio_current_song = kod;
		} else if (strcmp(key, "tradio_delete_song") == 0){
			komande.tradio_delete_song = kod;
		} else if (strcmp(key, "tradio_delete_all") == 0){
			komande.tradio_delete_all = kod;
		} else if (strcmp(key, "tradio_first_song") == 0){
			komande.tradio_first_song = kod;
		} else if (strcmp(key, "tradio_last_song") == 0){
			komande.tradio_last_song = kod;
		} else if (strcmp(key, "tradio_exit") == 0){
			komande.tradio_exit = kod;
		}
	}
	fclose(f);
}
void ucitaj_komande(void) {
	struct stat st;
	if (stat(komande_programa_putanja, &st) != 0) {
		napravi_default_komande();
	}
	FILE *f = fopen(komande_programa_putanja, "r");
	if (!f){
		return;
	}
	char line[128];
	while (fgets(line, sizeof(line), f)){
		if (line[0] == '#' || strlen(line) < 3){
			continue;
		}
		char key[64];
		char value[64];
		if (sscanf(line, "%63[^=]=%63s", key, value) != 2){
			continue;
		}
		int kod = tmuzika_parse_taster(value);
		if (kod < 0){
			continue;
		}
		// GLOBAL
		if (strcmp(key, "tmuzika_scroll_left") == 0) komande.tmuzika_scroll_left = kod;
		else if (strcmp(key, "tmuzika_scroll_right") == 0) komande.tmuzika_scroll_right = kod;
		else if (strcmp(key, "tmuzika_reset_key") == 0) komande.tmuzika_reset_key = kod;
		else if (strcmp(key, "tmuzika_reset_color") == 0) komande.tmuzika_reset_color = kod;
		// TMUZIKA
		else if (strcmp(key, "tmuzika_play") == 0) komande.tmuzika_play = kod;
		else if (strcmp(key, "tmuzika_pause") == 0) komande.tmuzika_pause = kod;
		else if (strcmp(key, "tmuzika_stop") == 0) komande.tmuzika_stop = kod;
		else if (strcmp(key, "tmuzika_next") == 0) komande.tmuzika_next = kod;
		else if (strcmp(key, "tmuzika_preview") == 0) komande.tmuzika_preview = kod;
		else if (strcmp(key, "tmuzika_volume_up") == 0) komande.tmuzika_volume_up = kod;
		else if (strcmp(key, "tmuzika_volume_down") == 0) komande.tmuzika_volume_down = kod;
		else if (strcmp(key, "tmuzika_seek_f") == 0) komande.tmuzika_seek_f = kod;
		else if (strcmp(key, "tmuzika_seek_b") == 0) komande.tmuzika_seek_b = kod;
		else if (strcmp(key, "tmuzika_repete_song") == 0) komande.tmuzika_repete_song = kod;
		else if (strcmp(key, "tmuzika_repete_all") == 0) komande.tmuzika_repete_all = kod;
		else if (strcmp(key, "tmuzika_shutlle") == 0) komande.tmuzika_shutlle = kod;
		else if (strcmp(key, "tmuzika_search") == 0) komande.tmuzika_search = kod;
		else if (strcmp(key, "tmuzika_radio") == 0) komande.tmuzika_radio = kod;
		else if (strcmp(key, "tmuzika_file_manager") == 0) komande.tmuzika_file_manager = kod;
		else if (strcmp(key, "tmuzika_save_m3u") == 0) komande.tmuzika_save_m3u = kod;
		else if (strcmp(key, "tmuzika_load_m3u") == 0) komande.tmuzika_load_m3u = kod;
		else if (strcmp(key, "tmuzika_current_song") == 0) komande.tmuzika_current_song = kod;
		else if (strcmp(key, "tmuzika_delete_song") == 0) komande.tmuzika_delete_song = kod;
		else if (strcmp(key, "tmuzika_delete_all") == 0) komande.tmuzika_delete_all = kod;
		else if (strcmp(key, "tmuzika_first_song") == 0) komande.tmuzika_first_song = kod;
		else if (strcmp(key, "tmuzika_last_song") == 0) komande.tmuzika_last_song = kod;
		else if (strcmp(key, "tmuzika_exit") == 0) komande.tmuzika_exit = kod;
		// TRADIO
		else if (strcmp(key, "tradio_play") == 0) komande.tradio_play = kod;
		else if (strcmp(key, "tradio_pause") == 0) komande.tradio_pause = kod;
		else if (strcmp(key, "tradio_stop") == 0) komande.tradio_stop = kod;
		else if (strcmp(key, "tradio_next") == 0) komande.tradio_next = kod;
		else if (strcmp(key, "tradio_preview") == 0) komande.tradio_preview = kod;
		else if (strcmp(key, "tradio_volume_up") == 0) komande.tradio_volume_up = kod;
		else if (strcmp(key, "tradio_volume_down") == 0) komande.tradio_volume_down = kod;
		else if (strcmp(key, "tradio_add_state_f") == 0) komande.tradio_add_state_f = kod;
		else if (strcmp(key, "tradio_search") == 0) komande.tradio_search = kod;
		else if (strcmp(key, "tradio_file_manager") == 0) komande.tradio_file_manager = kod;
		else if (strcmp(key, "tradio_add") == 0) komande.tradio_load_m3u = kod;
		else if (strcmp(key, "tradio_save_m3u") == 0) komande.tradio_save_m3u = kod;
		else if (strcmp(key, "tradio_load_m3u") == 0) komande.tradio_load_m3u = kod;
		else if (strcmp(key, "tradio_current_song") == 0) komande.tradio_current_song = kod;
		else if (strcmp(key, "tradio_delete_song") == 0) komande.tradio_delete_song = kod;
		else if (strcmp(key, "tradio_delete_all") == 0) komande.tradio_delete_all = kod;
		else if (strcmp(key, "tradio_first_song") == 0) komande.tradio_first_song = kod;
		else if (strcmp(key, "tradio_last_song") == 0) komande.tradio_last_song = kod;
		else if (strcmp(key, "tradio_exit") == 0) komande.tradio_exit = kod;
	}
	fclose(f);
}
void tmuzika_reset_komande_prompt(){
	werase(stdscr);
	refresh();
	wattron(stdscr, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
	box(stdscr, 0, 0);
	wattroff(stdscr, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
	mvprintw(1, 1, _("Are you sure you want to reset command configuration? (y/N)"));
	refresh();
	wtimeout(stdscr, -1);
	int c = getch();
	wtimeout(stdscr, 200);
	if (c == 'y' || c == 'Y') {
		remove(komande_programa_putanja);
		napravi_default_komande();
		komande_programa_konfiguracija1();
		ucitaj_komande();
	} else {
		cpu_ciscenje = 1;
	}
}