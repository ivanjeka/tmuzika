/*
 * tmuzika - Terminal music player
 * Copyright (C) 2026 Ivan Janković
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
#include <wchar.h>
#include <libintl.h>
#include <getopt.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>
#include <time.h>
#include "tmuzika_status.h"
#include "tmuzika.h"
#include "tradio_status.h"
#include "tradio.h"
#include "definicije.h"
#include "filemanager.h"

// TRADIO STATUS
TradioStatus tradio_status = TRADIO_ZAUSTAVLJEN;
char tradio_title[256] = "";
char tradio_artist[128] = "";
char tradio_album[128] = "";
int tradio_track = 0;
int tradio_elapsed = 0;
int tradio_length = 0;
// TMUZIKA STATUS
TmuzikaStatus tmuzika_status = TMUZIKA_ZAUSTAVLJEN;
char tmuzika_title[256] = "";
char tmuzika_artist[128] = "";
char tmuzika_album[128] = "";
int tmuzika_track = 0;
int tmuzika_elapsed = 0;
int tmuzika_length = 0;
// TRADIO
static RadioLista radio;
static char radio_fajl[PATH_MAX];
static int radio_prozor = 0;
static GstElement *radio_bin = NULL;
static char poslednja_pustena_stanica[MAX_DUZINA];
static time_t poslednja_sekunda = 0;
// TMUZIKA
static GstElement *reprodukcioni_blok = NULL;
static GstBus *poruke = NULL;
static Lista_pesama playlist;
static int pokretanje = 1;
static int pustanje = 0;
static int pauziranje = 0;
static char konfiguracioni_folder[MAX_DUZINA];
static char putanja_osnovne_liste[MAX_DUZINA];
static time_t vreme_pesme_sekunda = 0;
static char poslednja_pustena_pesma[MAX_DUZINA];
static int utf8_ok = 0;
static int lista_pocetni_red = 0;
static int lista_prikazivanje = 0;
// Ponavljanje i nasumicno
static int ponovi_pesmu = 0;
static int ponovi_listu = 0;
static int nasumicno_pustanje = 0;
static int *nasumicno_red = NULL;
static int nasumicno_velicina = 0;
static int nasumicno_pozicija = 0;

static gint64 cached_pos = GST_CLOCK_TIME_NONE;
static gint64 cached_dur = GST_CLOCK_TIME_NONE;
static int horizontal_scroll = 0;
static int max_line_length = 0;
// FOOTER
static char footer_text[256] = "";
static gboolean footer_privremeni = FALSE;
static gint64 footer_vreme = 0;   // u mikrosekundama
// CLI
static int cli_mode = 0;
static char cli_start_path[PATH_MAX] = "";
// MENADZER DATOTEKA
static char *bookmarks[MAX_BOOKMARKS] ={NULL};
static gboolean fm_prikazi_skrivene = FALSE;
char putanja_cache_liste[PATH_MAX];
static enum Rezim trenutni_rezim = REZIM_PLAYER;
// CPU CIscenje
static int cpu_ciscenje = 1;
int radio_cpu_ciscenje = 1;
static gboolean fm_cpu_ciscenje = FALSE;

// POZIVANJE FUNKCIJA
static guint ucitavanje_m3u_liste_radio(const char *path);
static void glava(WINDOW *win);
static void sacuvaj_m3u_liste_radio();
static void radio_zaustavi();
static void zaustavi_pesmu();
static void pomoc_komande(WINDOW *win);
static int zahtev_unosa_esc(WINDOW *win, const char *prompt, char *out, size_t outlen);
static void izvuci_artist_album_title(const char *uri, char *artist, size_t a_len, char *album, size_t al_len, char *title, size_t t_len);

static int fajl_postoji(const char *path){
	struct stat st;
	return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}
static void formatiraj_vreme(int sekunde, char *out, size_t outlen){
    int m = sekunde / 60;
    int s = sekunde % 60;
    snprintf(out, outlen, "%02d:%02d", m, s);
}
static void pocetna_lista(Lista_pesama *pl){
	pl->stavke = g_ptr_array_new_with_free_func(g_free);
	pl->backup_stavke = NULL;
	pl->trenutna = 0;
	pl->svira = -1;
	pl->volume = 1.00;
}
static void oslobadjanje_liste(Lista_pesama *pl){
	if (pl->stavke){
		g_ptr_array_free(pl->stavke, TRUE);
		pl->stavke = NULL;
	}
}
static void dodavanje_putanje_u_listu(Lista_pesama *pl, const char *uri){
	if (!uri) return;
	g_ptr_array_add(pl->stavke, g_strdup(uri));
}
static int dodavanje_fajla_u_listu(Lista_pesama *pl, const char *filename){
	if (!fajl_postoji(filename)){
		return 0;
	}
	char *uri = gst_filename_to_uri(filename, NULL);
	if (!uri){
		return 0;
	}
	dodavanje_putanje_u_listu(pl, uri);
	g_free(uri);
	return 1;
}
static int provera_da_li_je_audio_fajl(const char *name){
	const char *ext = strrchr(name, '.');
	if (!ext) return 0;
	ext++;
	if (!ext) return 0;
	if (g_ascii_strcasecmp(ext, "mp3") == 0) return 1;
	if (g_ascii_strcasecmp(ext, "wav") == 0) return 1;
	if (g_ascii_strcasecmp(ext, "flac") == 0) return 1;
	if (g_ascii_strcasecmp(ext, "ogg") == 0) return 1;
	if (g_ascii_strcasecmp(ext, "m4a") == 0) return 1;
	if (g_ascii_strcasecmp(ext, "aac") == 0) return 1;
	if (g_ascii_strcasecmp(ext, "opus") == 0) return 1;
	return 0;
}
static int folder_ima_audio_fajlove(const char *putanja){
	DIR *dir = opendir(putanja);
	if (!dir){
		return 0;
	}
	struct dirent *ent;
	while ((ent = readdir(dir))){
		if (ent->d_name[0] == '.'){
			continue;
		}
		char *puna_putanja = g_build_filename(putanja, ent->d_name, NULL);
		struct stat st;
		if (stat(puna_putanja, &st) == 0){
			if (S_ISREG(st.st_mode)){
				if (provera_da_li_je_audio_fajl(ent->d_name)){
					g_free(puna_putanja);
					closedir(dir);
					return 1;
				}
			}
			else if (S_ISDIR(st.st_mode)){
				if (folder_ima_audio_fajlove(puna_putanja)){
					g_free(puna_putanja);
					closedir(dir);
					return 1;
				}
			}
		}
		g_free(puna_putanja);
	}
	closedir(dir);
	return 0;
}
static int dodavanje_celog_foldera(Lista_pesama *pl, const char *folder){
	DIR *d = opendir(folder);
	if (!d){
		return 0;
	}
	struct dirent *entry;
	int dodato = 0;
	while ((entry = readdir(d)) != NULL){
		if (strcmp(entry->d_name, ".") == 0 ||
			strcmp(entry->d_name, "..") == 0){
			continue;
		}
		char path[MAX_DUZINA];
		snprintf(path, sizeof(path), "%s/%s", folder, entry->d_name);
		struct stat st;
		if (stat(path, &st) != 0){
			continue;
		}
		if (S_ISDIR(st.st_mode)){
			if (dodavanje_celog_foldera(pl, path)){
				dodato = 1;
			}
		} else if (S_ISREG(st.st_mode) &&
			provera_da_li_je_audio_fajl(entry->d_name)){
			if (dodavanje_fajla_u_listu(pl, path)){
				dodato = 1;
			}
		}
	}
	closedir(d);
	return dodato;
}
static void sacuvaj_poslednju_pesmu(){
	FILE *f = fopen(poslednja_pustena_pesma, "w");
	if (!f){
		return;
	}
	fprintf(f, "%d\n", playlist.trenutna);
	fclose(f);
}
static void ucitaj_poslednju_pesmu(){
	FILE *f = fopen(poslednja_pustena_pesma, "r");
	if (!f){
		return;
	}
	int idx = 0;
	if (fscanf(f, "%d", &idx) == 1){
		if (playlist.stavke && idx >= 0 && idx < (int)playlist.stavke->len)
		playlist.trenutna = idx;
	}
	fclose(f);
}
static void init_konfiguracioni_folder(){
	const char *home = getenv("HOME");
	if (!home){
		home = ".";
	}
	snprintf(konfiguracioni_folder, sizeof(konfiguracioni_folder), "%s/.tmuzika", home);
	struct stat st ={0};
	if (stat(konfiguracioni_folder, &st) == -1){
		mkdir(konfiguracioni_folder, 0700);
	}
	size_t need = strlen(konfiguracioni_folder) + 1 + strlen("glavna.m3u") + 1;
	if (need < sizeof(putanja_osnovne_liste)){
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wformat-truncation"
		snprintf(putanja_osnovne_liste, sizeof(putanja_osnovne_liste), "%s/glavna.m3u", konfiguracioni_folder);
		#pragma GCC diagnostic pop
	} else{
		strncpy(putanja_osnovne_liste, "glavna.m3u", sizeof(putanja_osnovne_liste));
		putanja_osnovne_liste[sizeof(putanja_osnovne_liste)-1] = '\0';
	}
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-truncation"
	snprintf(poslednja_pustena_pesma, sizeof(poslednja_pustena_pesma), "%s/poslednje_slusano", konfiguracioni_folder);
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-truncation"
	snprintf(putanja_cache_liste, sizeof(putanja_cache_liste), "%s/glavna.cache", konfiguracioni_folder);
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-truncation"
	snprintf(radio_fajl, sizeof(radio_fajl), "%s/radio.tradio", konfiguracioni_folder);
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wformat-truncation"
	snprintf(poslednja_pustena_stanica, sizeof(poslednja_pustena_stanica), "%s/poslednje_slusana_stanica", konfiguracioni_folder);
	#pragma GCC diagnostic pop

}
static void skracena_putanja_foldera(char *path, size_t size){
	if (path[0] == '~' && (path[1] == '/' || path[1] == '\0')){
		const char *home = getenv("HOME");
		if (!home){
			return;
		}
		char tmp[MAX_DUZINA];
		snprintf(tmp, sizeof(tmp), "%s%s", home, path + 1);
		strncpy(path, tmp, size - 1);
		path[size - 1] = '\0';
	}
}
static void sacuvaj_playlist_cache(Lista_pesama *pl){
	if (!pl || !pl->stavke || pl->stavke->len == 0){
		return;
	}
	FILE *f = fopen(putanja_cache_liste, "wb");
	if (!f){
		return;
	}
	uint32_t magic = TMUZIKA_CACHE_MAGIC;
	uint32_t version = TMUZIKA_CACHE_VERSION;
	uint32_t count = pl->stavke->len;
	fwrite(&magic, sizeof(magic), 1, f);
	fwrite(&version, sizeof(version), 1, f);
	fwrite(&count, sizeof(count), 1, f);
	for (guint i = 0; i < pl->stavke->len; i++){
		const char *uri = g_ptr_array_index(pl->stavke, i);
		gchar *path = gst_uri_get_location(uri);
		if (!path){
			continue;
		}
		uint32_t len = strlen(path) + 1;
		fwrite(&len, sizeof(len), 1, f);
		fwrite(path, 1, len, f);
		g_free(path);
	}
	fclose(f);
}
static gboolean ucitaj_playlist_cache(Lista_pesama *pl){
	FILE *f = fopen(putanja_cache_liste, "rb");
	if (!f){
		return FALSE;
	}
	uint32_t magic = 0, version = 0, count = 0;
	if (fread(&magic, sizeof(magic), 1, f) != 1 ||
		fread(&version, sizeof(version), 1, f) != 1 ||
		fread(&count, sizeof(count), 1, f)){
		fclose(f);
		return FALSE;
	}
	if (magic != TMUZIKA_CACHE_MAGIC || version != TMUZIKA_CACHE_VERSION){
		fclose(f);
		return FALSE;
	}
	for (uint32_t i = 0; i < count; i++){
		uint32_t len = 0;
		if (fread(&len, sizeof(len), 1, f) != 1 || len == 0 || len > 8192){
			fclose(f);
			return FALSE;
		}
		char *buf = g_malloc(len);
		if (fread(buf, 1, len, f) != len){
			g_free(buf);
			fclose(f);
			return FALSE;
		}
		char *uri = NULL;
		if (strstr(buf, "://")){
			uri = g_strdup(buf);
		} else{
			uri = gst_filename_to_uri(buf, NULL);
		}
		if (uri){
			dodavanje_putanje_u_listu(pl, uri);
			g_free(uri);
		}
		g_free(buf);
	}
	fclose(f);
	return TRUE;
}
static gboolean cache_validan(const char *m3u_path){
	struct stat st_m3u, st_cache;
	if (stat(m3u_path, &st_m3u) != 0){
		return FALSE;
	}
	if (stat(putanja_cache_liste, &st_cache) != 0){
		return FALSE;
	}
	//ako je cache noviji od m3u → validan 
	return st_cache.st_mtime >= st_m3u.st_mtime;
}
static void cuvanje_m3u_liste(Lista_pesama *pl, const char *outpath){
	char path[MAX_DUZINA];
	strncpy(path, outpath, sizeof(path) - 1);
	path[sizeof(path) - 1] = '\0';
	skracena_putanja_foldera(path, sizeof(path));
	const char *ext = strrchr(path, '.');
	if (!ext || g_ascii_strcasecmp(ext, ".m3u") != 0){
		if (strlen(path) + 4 < sizeof(path)){
			strcat(path, ".m3u");
		}
	}
	FILE *f = fopen(path, "w");
	if (!f){
		return;
	}
	fprintf(f, "#EXTM3U\n");
	for (guint i = 0; i < pl->stavke->len; ++i){
		const char *uri = g_ptr_array_index(pl->stavke, i);
		gchar *f_ime = gst_uri_get_location(uri);
		if (f_ime){
			fprintf(f, "%s\n", f_ime);
			g_free(f_ime);
		} else{
			fprintf(f, "%s\n", uri);
		}
	}
	fclose(f);
}
static void str_trim(char *s){
	if (!s){
		return;
	}		
	char *p = s;
	while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')){
		p++;
	}
	if (p != s){
		memmove(s, p, strlen(p) + 1);
	}
	size_t len = strlen(s);
	while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\r' || s[len-1] == '\n')){
		s[len-1] = '\0';
		len--;
	}
}
static void join_path(const char *base_dir, const char *rel, char *out, size_t outlen){
	if (!base_dir || !rel || !out){
		return;
	}
	if (rel[0] == '/'){ // absolute
		strncpy(out, rel, outlen-1);
		out[outlen-1] = '\0';
		return;
	}
	if (base_dir[0] == '\0'){
		strncpy(out, rel, outlen-1);
		out[outlen-1] = '\0';
		return;
	}
	size_t bl = strlen(base_dir);
	if (bl + 1 + strlen(rel) + 1 > outlen){
		strncpy(out, rel, outlen-1);
		out[outlen-1] = '\0';
		return;
	}
	if (base_dir[bl-1] == '/'){
		snprintf(out, outlen, "%s%s", base_dir, rel);
	} else{
		snprintf(out, outlen, "%s/%s", base_dir, rel);
	}
}
static guint ucitavanje_m3u_liste(Lista_pesama *pl, const char *inpath){
	guint count = 0;
	if (!pl || !inpath){
		return 0;
	}
	char playlist_path[MAX_DUZINA];
	strncpy(playlist_path, inpath, sizeof(playlist_path) - 1);
	playlist_path[sizeof(playlist_path) - 1] = '\0';
	skracena_putanja_foldera(playlist_path, sizeof(playlist_path));
	if (cache_validan(playlist_path)){
		if (ucitaj_playlist_cache(pl)){
			return 0;
		}
	}

	FILE *f = fopen(playlist_path, "r");
	if (!f){
		perror("fopen m3u");
		return 0;
	}
	char base_dir[MAX_DUZINA] ={0};
	char *copy = g_strdup(playlist_path);
	char *last_slash = strrchr(copy, '/');
	if (last_slash){
		*last_slash = '\0';
		strncpy(base_dir, copy, sizeof(base_dir)-1);
	} else{
		base_dir[0] = '\0';
	}
	g_free(copy);
	char line[MAX_DUZINA];
	while (fgets(line, sizeof(line), f)){
		str_trim(line);
		if (line[0] == '\0'){
			continue;
		}
		if (line[0] == '#'){
			continue;
		}
		if (strstr(line, "://") != NULL){
			continue;
		}
		char resolved[MAX_DUZINA];
		strncpy(resolved, line, sizeof(resolved) - 1);
		resolved[sizeof(resolved) - 1] = '\0';
		skracena_putanja_foldera(resolved, sizeof(resolved));
		char fullpath[MAX_DUZINA] ={0};
		if (resolved[0] == '/'){
			strncpy(fullpath, resolved, sizeof(fullpath)-1);
		} else{
			join_path(base_dir, resolved, fullpath, sizeof(fullpath));
		}
		char *uri = gst_filename_to_uri(fullpath, NULL);
		if (uri){
			dodavanje_putanje_u_listu(pl, uri);
			count++; 
			g_free(uri);
		}
	}
	fclose(f);
	sacuvaj_playlist_cache(pl);
	return count;
}
static void jacina_zvuka(gdouble vol){
	if (!reprodukcioni_blok){
		return;
	}
	if (vol < 0.0){
		vol = 0.0;
	}
	if (vol > 1.00){
		vol = 1.00; // allow headroom
	}
	playlist.volume = vol;
	g_object_set(reprodukcioni_blok, "volume", vol, NULL);
}
static void sacuvaj_tmuzika_status(void){
	const char *title;
	const char *artist;
	const char *album;
	int track;
	int elapsed;
	int length;
	if (radio_prozor){
		title   = tradio_title;
		artist  = tradio_artist;
		album   = tradio_album;
		track   = tradio_track;
		elapsed = tradio_elapsed;
		length  = tradio_length;
	} else{
		title   = tmuzika_title;
		artist  = tmuzika_artist;
		album   = tmuzika_album;
		track   = tmuzika_track;
		elapsed = tmuzika_elapsed;
		length  = tmuzika_length;
	}
	char path[PATH_MAX];
	FILE *f;
	snprintf(path, sizeof(path), "%s/tmuzika_title", TMUZIKA_TMP_DIR);
	if ((f = fopen(path, "w"))){ 
		fprintf(f, "%s\n", title); 
		fclose(f); 
	}
	snprintf(path, sizeof(path), "%s/tmuzika_artist", TMUZIKA_TMP_DIR);
	if ((f = fopen(path, "w"))){ 
		fprintf(f, "%s\n", artist); 
		fclose(f); 
	}
	snprintf(path, sizeof(path), "%s/tmuzika_album", TMUZIKA_TMP_DIR);
	if ((f = fopen(path, "w"))){ 
		fprintf(f, "%s\n", album); 
		fclose(f); 
	}
	snprintf(path, sizeof(path), "%s/tmuzika_track", TMUZIKA_TMP_DIR);
	if ((f = fopen(path, "w"))){ 
		fprintf(f, "%d\n", track); 
		fclose(f); 
	}
	char elapsed_str[16];
	char length_str[16];
	formatiraj_vreme(elapsed, elapsed_str, sizeof(elapsed_str));
	formatiraj_vreme(length,  length_str,  sizeof(length_str));
	snprintf(path, sizeof(path), "%s/tmuzika_elapsed", TMUZIKA_TMP_DIR);
	if ((f = fopen(path, "w"))){ 
		fprintf(f, "%s\n", elapsed_str); 
		fclose(f); 
	}
	snprintf(path, sizeof(path), "%s/tmuzika_length", TMUZIKA_TMP_DIR);
	if ((f = fopen(path, "w"))){ 
		fprintf(f, "%s\n", length_str); 
		fclose(f); 
	}
}
static void pusti_izabranu_pesmu(){
	tmuzika_status = TMUZIKA_PUSTEN;
	if (!reprodukcioni_blok || playlist.stavke->len == 0){
		return;
	}
	if (playlist.trenutna < 0){
		playlist.trenutna = 0;
	}
	if (playlist.trenutna >= (int)playlist.stavke->len){
		playlist.trenutna = playlist.stavke->len - 1;
	}
	if (radio_bin && radio.online){
		gst_element_set_state(radio_bin, GST_STATE_NULL);
		radio.online = 0;
		radio.svira = -1;
	}
	gst_element_set_state(reprodukcioni_blok, GST_STATE_NULL);
	const char *uri = g_ptr_array_index(playlist.stavke, playlist.trenutna);
	g_object_set(reprodukcioni_blok, "uri", uri, NULL);
	playlist.svira = playlist.trenutna;
	gst_element_set_state(reprodukcioni_blok, GST_STATE_PLAYING);
	tmuzika_elapsed = 0;
	tmuzika_length = 0;
	tmuzika_track = playlist.trenutna + 1;
	if (uri){
		izvuci_artist_album_title(uri,
			tmuzika_artist, sizeof(tmuzika_artist),
			tmuzika_album, sizeof(tmuzika_album),
			tmuzika_title, sizeof(tmuzika_title));
		tmuzika_track   = playlist.trenutna + 1;
		tmuzika_elapsed = 0;
		tmuzika_length  = 0;
	}
	tmuzika_status = TMUZIKA_PUSTEN;
	sacuvaj_tmuzika_status();
	pustanje = 1;
	pauziranje = 0;
	cpu_ciscenje = 1;
	sacuvaj_poslednju_pesmu();	
}
static void zaustavi_pesmu(){
	if (radio_prozor){
		return;
	}
	if (!reprodukcioni_blok){
		return;
	}
	gst_element_set_state(reprodukcioni_blok, GST_STATE_READY);
	pustanje = 0;
	pauziranje = 0;
	playlist.svira = -1;
	cpu_ciscenje = 1;
	tmuzika_status = TMUZIKA_ZAUSTAVLJEN;
	sacuvaj_tmuzika_status();
}
static void pauziraj_pesmu(){
	if (radio_prozor){
		return;
	}
	if (!reprodukcioni_blok){
		return;
	}
	GstState cur = GST_STATE_NULL;
	gst_element_get_state(reprodukcioni_blok, &cur, NULL, 0);
	if (cur == GST_STATE_PLAYING){
		gst_element_set_state(reprodukcioni_blok, GST_STATE_PAUSED);
		pauziranje = 1;
	} else if (cur == GST_STATE_PAUSED){
		gst_element_set_state(reprodukcioni_blok, GST_STATE_PLAYING);
		pauziranje = 0;
	} else{
		pusti_izabranu_pesmu();
			tmuzika_status = TMUZIKA_PUSTEN;
	}
	tmuzika_status = TMUZIKA_PAUZIRAN;
	sacuvaj_tmuzika_status();
	cpu_ciscenje = 1;
}
static void sledeca_pesma(){
	if (radio_prozor){
		return;
	}
	if (playlist.stavke->len == 0){
		return;
	}
	gst_element_set_state(reprodukcioni_blok, GST_STATE_READY);
	playlist.trenutna++;
	if (playlist.trenutna >= (int)playlist.stavke->len){
		playlist.trenutna = 0;
	}
	pusti_izabranu_pesmu();
}
static void prethodna_pesma(){
	if (radio_prozor){
		return;
	}
	if (playlist.stavke->len == 0){
		return;
	}
	gst_element_set_state(reprodukcioni_blok, GST_STATE_READY);
	playlist.trenutna--;
	if (playlist.trenutna < 0){
		playlist.trenutna = playlist.stavke->len - 1;
	}
	pusti_izabranu_pesmu();
}
static void napravi_nasumican_red(){
	int n = playlist.stavke->len;
	if (n <= 0){
		return;
	}
	free(nasumicno_red);
	nasumicno_red = malloc(sizeof(int) * n);
	nasumicno_velicina = n;
	nasumicno_pozicija = 0;
	for (int i = 0; i < n; i++){
		nasumicno_red[i] = i;
	}
	for (int i = n - 1; i > 0; i--){
		int j = rand() % (i + 1);
		int tmp = nasumicno_red[i];
		nasumicno_red[i] = nasumicno_red[j];
		nasumicno_red[j] = tmp;
	}
}
static void sledeca_nasumicna_pesma(){
	if (nasumicno_pozicija >= nasumicno_velicina){
		napravi_nasumican_red();
	}
	playlist.trenutna = nasumicno_red[nasumicno_pozicija];
	nasumicno_pozicija++;
	pusti_izabranu_pesmu();
}
static void provera_zavrsetka_pesme(){
	if (radio_prozor){
		return;
	}
	if (!reprodukcioni_blok){
		return;
	}
	if (pauziranje){
		return;
	}
	GstMessage *msg = gst_bus_pop(poruke);
	while (msg){
		if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS){
			if (ponovi_pesmu){
				pusti_izabranu_pesmu();
			} else if (nasumicno_pustanje){
				sledeca_nasumicna_pesma();
			} else{
				sledeca_pesma();
			}
		}
		gst_message_unref(msg);
		msg = gst_bus_pop(poruke);
	}
}
static void ubrzaj_pesmu(gint64 offset_ns){
	if (!reprodukcioni_blok){
		return;
	}
	gint64 pos = GST_CLOCK_TIME_NONE;
	if (!gst_element_query_position(reprodukcioni_blok, GST_FORMAT_TIME, &pos)){
		return;
	}
	gint64 target = pos + offset_ns;
	if (target < 0){
		target = 0;
	}
	gst_element_seek_simple(
		reprodukcioni_blok,
		GST_FORMAT_TIME,
		GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
		target
	);
	cpu_ciscenje = 1;
}
void isprazni_playlistu(Lista_pesama *pl){
	if (!pl){
		return;
	}
	if (!pl->stavke){
		pocetna_lista(pl);
		pl->trenutna = 0;
		return;
	}
	g_ptr_array_set_size(pl->stavke, 0);
	pl->trenutna = 0;
	zaustavi_pesmu();
	cpu_ciscenje = 1;
}
void ukloni_trenutnu_pesmu(Lista_pesama *pl){
	if (!pl || !pl->stavke){
		return;
	}
	if (pl->stavke->len == 0){
		return;
	}
	guint idx = pl->trenutna;
	g_ptr_array_remove_index(pl->stavke, idx);
	if (pl->trenutna >= (int)pl->stavke->len){
		pl->trenutna = pl->stavke->len - 1;
	}
	if (pl->trenutna < 0){
		pl->trenutna = 0;
	}
	if (pl->stavke->len == 0){
		zaustavi_pesmu();
		pl->trenutna = 0;
	}
	cpu_ciscenje = 1;
}
static void prikazivanje_vremena(gint64 ns, char *out, size_t outlen){
	if (ns == GST_CLOCK_TIME_NONE){ 
		strncpy(out, "--:--", outlen); 
		return; 
	}
	gint64 secs = ns / GST_SECOND;
	gint64 m = secs / 60;
	gint64 s = secs % 60;
	snprintf(out, outlen, "%lld:%02lld", (long long)m, (long long)s);
}
static void vrati_playlistu_i_pozicioniraj(Lista_pesama *pl, const char *trenutni_uri){
	if (!pl->backup_stavke || !trenutni_uri){
		return;
	}
	GPtrArray *full = pl->backup_stavke;
	int found = 0;
	for (guint i = 0; i < full->len; i++){
		const char *uri = g_ptr_array_index(full, i);
		if (g_strcmp0(uri, trenutni_uri) == 0){
			found = i;
			break;
		}
	}
	g_ptr_array_free(pl->stavke, TRUE);
	pl->stavke = full;
	pl->backup_stavke = NULL;
	pl->trenutna = found;
	pl->svira = found;
}
static void footer_lista(WINDOW *win){
	int h, w;
	getmaxyx(win, h, w);
	int fy = h - 1;
	mvwprintw(win, fy, 0, "%*s", w, "");
	const char *tekst; 
	if (radio_prozor){
		tekst = _(" ENTER/SPACE:play/pause z:stop ↑↓:move x:remove m:manager t:search ESC:quit ");
	} else{
		tekst = _(" ENTER/SPACE:play/pause z:stop ↑↓:move x:remove m:manager t:search k:exit ");
	}

	wchar_t wtekst[256];
	mbstowcs(wtekst, tekst, sizeof(wtekst)/sizeof(wchar_t));
	int len = wcswidth(wtekst, wcslen(wtekst));
	int start_linije = 2 + len;
	if (start_linije >= w - 1){
		start_linije = w - 2;
	}
	int duzina_linije = w - 1 - start_linije;
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	mvwprintw(win, fy, 0, "└─");
	mvwprintw(win, fy, 2, "%s", tekst);
	if (duzina_linije > 0){
		mvwhline(win, fy, start_linije, ACS_HLINE, duzina_linije);
	}
	mvwprintw(win, fy, w - 1, "┘");
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	wrefresh(win);
}
static void footer_unos(WINDOW *win){
	int h, w;
	getmaxyx(win, h, w);
	int y = h - 2;
	int fy = h - 1;
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	mvhline(y, 1, ACS_HLINE, w - 2);
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	mvwprintw(win, fy, 2, _(" ENTER:confirm  ESC:quit "));
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
}
static void footer_pomoc(WINDOW *win){
	int h, w;
	getmaxyx(win, h, w);
	int y = h - 2;
	int fy = h - 1;
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	mvhline(y, 1, ACS_HLINE, w - 2);
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	mvwprintw(win, fy, 2, _(" ESC:quit ↑↓:scroll bottom panel PgUp/PgDn:scroll top panel "));
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
}
static void footer_poruka(WINDOW *win){
	int h, w;
	getmaxyx(win, h, w);
	int fy = h - 1;
	mvwprintw(win, fy, 0, "%*s", w, ""); 
	wchar_t wtekst[256];
	mbstowcs(wtekst, footer_text, sizeof(wtekst)/sizeof(wchar_t));
	int len = wcswidth(wtekst, wcslen(wtekst));
	int start_linije = 2 + len;
	if (start_linije >= w - 1){
		start_linije = w - 2;
	}
	int duzina_linije = w - 1 - start_linije;
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	mvwprintw(win, fy, 0, "└─");
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	wattron(win, COLOR_PAIR(ZELENA_BOJA));
	mvwprintw(win, fy, 2, "%s", footer_text);
	wattroff(win, COLOR_PAIR(ZELENA_BOJA));
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	if (duzina_linije > 0){
		mvwhline(win, fy, start_linije, ACS_HLINE, duzina_linije);
	}
	mvwprintw(win, fy, w - 1, "┘");
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	wrefresh(win);
}
static void footer_datoteka(WINDOW *win){
	int h, w;
	getmaxyx(win, h, w);
	int fy = h - 1;
	mvwprintw(win, fy, 0, "%*s", w, "");
	const char *tekst = _(" ENTER:open/add  BACKSPACE:back  f:add folder  u:load m3u  ESC:quit ");
	wchar_t wtekst[256];
	mbstowcs(wtekst, tekst, sizeof(wtekst)/sizeof(wchar_t));
	int len = wcswidth(wtekst, wcslen(wtekst));
	int start_linije = 2 + len;
	if (start_linije >= w - 1){
		start_linije = w - 2;
	}
	int duzina_linije = w - 1 - start_linije;
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	mvwprintw(win, fy, 0, "└─");
	mvwprintw(win, fy, 2, "%s", tekst);
	if (duzina_linije > 0){
		mvwhline(win, fy, start_linije, ACS_HLINE, duzina_linije);
	}
	mvwprintw(win, fy, w - 1, "┘");
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	wrefresh(win);
}
static void footer_prikaz_poruka(WINDOW *win, const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(footer_text, sizeof(footer_text), fmt, ap);
	va_end(ap);
	footer_privremeni = TRUE;
	footer_vreme = g_get_monotonic_time();
	footer_poruka(win);
}
static void footer_osvezavanje(WINDOW *win){
	if (!footer_privremeni){
		return;
	}
	gint64 sada = g_get_monotonic_time();
	if (sada - footer_vreme >= 2 * G_USEC_PER_SEC){
		footer_privremeni = FALSE;
		footer_datoteka(win);
	}
}
static const char *fm_bookmark_path(void){
	static char path[PATH_MAX];
	const char *home = g_get_home_dir();
	snprintf(path, sizeof(path), "%s/.tmuzika/fm_bookmarks", home);
	return path;
}
static void fm_bookmarks_load(void){
	FILE *f;
	char line[PATH_MAX];
	int i = 0;
	const char *path = fm_bookmark_path();
	f = fopen(path, "r");
	if (!f){
		return;
	}
	while (fgets(line, sizeof(line), f) && i < MAX_BOOKMARKS){
		line[strcspn(line, "\n")] = 0;
		if (line[0] != '\0'){
			bookmarks[i] = g_strdup(line);
		}
		i++;
	}
	fclose(f);
}
static void fm_bookmarks_save(void){
	FILE *f;
	int i;
	const char *path = fm_bookmark_path();
	g_mkdir_with_parents(g_path_get_dirname(path), 0700);
	f = fopen(path, "w");
	if (!f){
		return;
	}
	for (i = 0; i < MAX_BOOKMARKS; i++){
		if (bookmarks[i]){
			fprintf(f, "%s\n", bookmarks[i]);
		} else{
			fprintf(f, "\n");
		}
	}
	fclose(f);
}
static int fm_prompt_bookmark_index(WINDOW *win){
	int ch;
	footer_prikaz_poruka(win, _(" Delete:bookmark (1–9), ESC:cancel "));
	while (1){
		ch = wgetch(win);
		if (ch == 27){
			return -1;
		}
		if (ch >= '1' && ch <= '9'){
			return ch - '1';
		}
	}
}
static void fm_bookmarks_compact(void){
	int i, j = 0;
	char *tmp[MAX_BOOKMARKS] ={0};
	for (i = 0; i < MAX_BOOKMARKS; i++){
		if (bookmarks[i]){
			tmp[j++] = bookmarks[i];
		}
	}
	memcpy(bookmarks, tmp, sizeof(tmp));
	fm_bookmarks_save();
}
static void chmod_ui(const char *path){
	struct stat st;
	if (stat(path, &st) != 0){ 
		return;
	}
	char input[4];
	echo();
	curs_set(1);
	footer_prikaz_poruka(stdscr, _(" Enter new permissions (octal, e.g. 755) for \"%s\": "), path);
	wgetnstr(stdscr, input, sizeof(input) - 1);
	noecho();
	curs_set(0);
	if (strlen(input) == 0){ 
		return;
	}
	long new_mode = strtol(input, NULL, 8);
	chmod(path, new_mode);
	footer_prikaz_poruka(stdscr, _(" Permissions updated to %o "), (unsigned int)new_mode);
}
static const char *nadji_editor(void){
	if (access("/usr/bin/nano", X_OK) == 0) return "nano";
	if (access("/bin/nano", X_OK) == 0) return "nano";
	if (access("/usr/bin/vim", X_OK) == 0) return "vim";
	if (access("/bin/vim", X_OK) == 0) return "vim";
	if (access("/usr/bin/vi", X_OK) == 0) return "vi";
	return NULL;
}
static gint poredjaj_foldere_abecedno(gconstpointer a, gconstpointer b){
	const char *sa = *(const char **)a;
	const char *sb = *(const char **)b;
	int a_dot = (sa[0] == '.');
	int b_dot = (sb[0] == '.');
	if (a_dot != b_dot){
		return b_dot - a_dot;
	}
	return g_ascii_strcasecmp(sa, sb);
}
static gint poredjaj_fajlove_abecedno(gconstpointer a, gconstpointer b){
	const char *fa = *(const char **)a;
	const char *fb = *(const char **)b;
	const char *ext_a = strrchr(fa, '.');
	const char *ext_b = strrchr(fb, '.');
	// AUDIO
	const char *audio_exts[] ={ ".mp3", ".flac", ".wav", ".ogg", ".m4a", ".aac", ".opus", NULL };
	int is_audio_a = 0, is_audio_b = 0;
	for (int i = 0; audio_exts[i]; i++){
		if (ext_a && g_ascii_strcasecmp(ext_a, audio_exts[i]) == 0){
			is_audio_a = 1;
		}
		if (ext_b && g_ascii_strcasecmp(ext_b, audio_exts[i]) == 0){
			is_audio_b = 1;
		}
	}
	if (is_audio_a != is_audio_b){
		return is_audio_b - is_audio_a;
	}	
	int is_m3u_a = ext_a && g_ascii_strcasecmp(ext_a, ".m3u") == 0;
	int is_m3u_b = ext_b && g_ascii_strcasecmp(ext_b, ".m3u") == 0;
	if (is_m3u_a != is_m3u_b){
		return is_m3u_b - is_m3u_a;
	}
	// VIDEO
	const char *video_exts[] ={ ".mp4", ".mkv", ".avi", ".mov", ".wmv", ".flv", ".webm", ".mpeg", ".mpg", ".m4v", ".3gp", ".ogv", ".ts", NULL };
	int is_video_a = 0, is_video_b = 0;
	for (int i = 0; video_exts[i]; i++){
		if (ext_a && g_ascii_strcasecmp(ext_a, video_exts[i]) == 0){
			is_video_a = 1;
		}
		if (ext_b && g_ascii_strcasecmp(ext_b, video_exts[i]) == 0){
			is_video_b = 1;
		}
	}
	if (is_video_a != is_video_b){
		return is_video_b - is_video_a;
	}
	// SLIKE
	const char *slika_exts[] ={ ".jpg", ".jpeg", ".png", ".bmp", ".webp", ".tif", ".tiff", ".svg", ".heif", ".heic", ".gif", NULL };
	int is_slika_a = 0, is_slika_b = 0;
	for (int i = 0; slika_exts[i]; i++){
		if (ext_a && g_ascii_strcasecmp(ext_a, slika_exts[i]) == 0){
			is_slika_a = 1;
		}
		if (ext_b && g_ascii_strcasecmp(ext_b, slika_exts[i]) == 0){
			is_slika_b = 1;
		}
	}
	if (is_slika_a != is_slika_b){
		return is_slika_b - is_slika_a;
	}
	// TEXT I OFFICE
	const char *office_exts[] ={ ".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx", ".csv", ".md", ".odt", ".ods", ".odp", ".pdf", ".epub", ".rtf", ".txt", NULL };
	int is_office_a = 0, is_office_b = 0;
	for (int i = 0; office_exts[i]; i++){
		if (ext_a && g_ascii_strcasecmp(ext_a, office_exts[i]) == 0){
			is_office_a = 1;
		}
		if (ext_b && g_ascii_strcasecmp(ext_b, office_exts[i]) == 0){
			is_office_b = 1;
		}
	}
	if (is_office_a != is_office_b){
		return is_office_b - is_office_a;
	}
	// PROGRAMSKI JEZICI
	const char *programski_jezici_exts[] ={ ".c", ".cpp", ".cc", ".cxx", ".cs", ".csx", ".xml", ".h", ".hpp", ".hh", ".py", ".pyc", ".pyd", ".so", ".rb", ".rbz", ".html", ".css", ".php", ".java", ".class", ".jar", ".js", ".cmd", ".btm", ".bat", ".exe", ".dll",".vb", ".vbp", ".vbs", ".vbg", ".frm", ".frx", ".bas", ".cls", ".clt", ".sql", ".db", ".go", ".rs", ".ts", ".tsx", ".mts", ".cts", NULL };
	int is_programski_jezici_a = 0, is_programski_jezici_b = 0;
	for (int i = 0; programski_jezici_exts[i]; i++){
		if (ext_a && g_ascii_strcasecmp(ext_a, programski_jezici_exts[i]) == 0){
			is_programski_jezici_a = 1;
		}
		if (ext_b && g_ascii_strcasecmp(ext_b, programski_jezici_exts[i]) == 0){
			is_programski_jezici_b = 1;
		}
	}
	if (is_programski_jezici_a != is_programski_jezici_b){
		return is_programski_jezici_b - is_programski_jezici_a;
	}
	// ARHIVA
	const char *archive_exts[] ={ ".zip", ".rar", ".7z", ".tar", ".gz", NULL };
	int is_archive_a = 0, is_archive_b = 0;
	for (int i = 0; archive_exts[i]; i++){
		if (ext_a && g_ascii_strcasecmp(ext_a, archive_exts[i]) == 0){
			is_archive_a = 1;
		}
		if (ext_b && g_ascii_strcasecmp(ext_b, archive_exts[i]) == 0){
			is_archive_b = 1;
		}
	}
	if (is_archive_a != is_archive_b){
		return is_archive_b - is_archive_a;
	}
	return g_ascii_strcasecmp(fa, fb);
}
static gboolean je_image_fajl(const char *ime){
	const char *ext = strrchr(ime, '.');
	if (!ext){
		return FALSE;
	}
	const char *imgs[] ={
		".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp",
		".tif", ".tiff", ".svg", ".heif", ".heic", NULL
	};
	for (int i = 0; imgs[i]; i++){
		if (g_ascii_strcasecmp(ext, imgs[i]) == 0){
			return TRUE;
		}
	}
	return FALSE;
}
static void otvori_image(const char *putanja){
	pid_t pid = fork();
	if (pid == 0){
		setsid();
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
		execlp("xdg-open", "xdg-open", putanja, (char *)NULL);
		execlp("feh", "feh", putanja, (char *)NULL);
		execlp("eog", "eog", putanja, (char *)NULL);
		execlp("display", "display", putanja, (char *)NULL);
		_exit(127);
	}
}
static gboolean je_video_fajl(const char *ime){
	const char *ext = strrchr(ime, '.');
	if (!ext){
		return FALSE;
	}
	const char *vids[] ={
		".mp4", ".mkv", ".avi", ".mov", ".wmv", ".flv", ".webm",
		".mpeg", ".mpg", ".m4v", ".3gp", ".ogv", ".ts", NULL
	};
	for (int i = 0; vids[i]; i++){
		if (g_ascii_strcasecmp(ext, vids[i]) == 0){
			return TRUE;
		}
	}
	return FALSE;
}
static void otvori_video(const char *putanja){
	if (!putanja || !*putanja){
		return;
	}
	pid_t pid = fork();
	if (pid == 0){
		setsid();
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
		execlp("vlc", "vlc", putanja, (char *)NULL);
		execlp("mpv", "mpv", putanja, (char *)NULL);
		execlp("smplayer", "smplayer", putanja, (char *)NULL);
		execlp("celluloid", "celluloid", putanja, (char *)NULL);
		execlp("totem", "totem", putanja, (char *)NULL);
		execlp("dragon", "dragon", putanja, (char *)NULL);
		execlp("parole", "parole", putanja, (char *)NULL);
		execlp("caja-open", "caja-open", putanja, (char *)NULL);
		execlp("xdg-open", "xdg-open", putanja, (char *)NULL);
		_exit(127);
	}
}
static gboolean je_office_fajl(const char *ime){
	const char *ext = strrchr(ime, '.');
	if (!ext){
		return FALSE;
	}
	const char *docs[] ={
        	".doc", ".docx", ".odt", ".rtf",
		".xls", ".xlsx", ".ods", ".csv",
		".ppt", ".pptx", ".odp", ".pdf",
		".md", ".epub",
		NULL
	};
	for (int i = 0; docs[i]; i++){
		if (g_ascii_strcasecmp(ext, docs[i]) == 0){
			return TRUE;
		}
	}
	return FALSE;
}
static void otvori_office(const char *putanja){
	if (!putanja || !*putanja){
		return;
	}
	pid_t pid = fork();
	if (pid == 0){
		setsid();
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
		execlp("libreoffice", "libreoffice", putanja, (char *)NULL);
		execlp("soffice", "soffice", putanja, (char *)NULL);
		execlp("openoffice", "openoffice", putanja, (char *)NULL);
		execlp("wps", "wps", putanja, (char *)NULL);
		execlp("wps-office", "wps-office", putanja, (char *)NULL);
		execlp("onlyoffice-desktopeditors", "onlyoffice-desktopeditors", putanja, (char *)NULL);
		execlp("calligra", "calligra", putanja, (char *)NULL);
		execlp("xdg-open", "xdg-open", putanja, (char *)NULL);
		_exit(127);
	}
}
static gboolean je_arhiva(const char *ime){
	const char *ext = strrchr(ime, '.');
	if (!ext){
		return FALSE;
	}
	const char *arh[] ={
		".zip", ".tar", ".gz", ".tgz", ".bz2", ".xz",
		".rar", ".7z", ".tar.gz", ".tar.bz2", ".tar.xz", NULL
	};
	for (int i = 0; arh[i]; i++){
		if (g_ascii_strcasecmp(ext, arh[i]) == 0){
			return TRUE;
		}
	}
	return FALSE;
}
static void otvori_u_arhiv_menedzeru(const char *putanja){
	pid_t pid = fork();
	if (pid == 0){
		execlp("file-roller", "file-roller", putanja, NULL);
		execlp("ark", "ark", putanja, NULL);
		execlp("engrampa", "engrampa", putanja, NULL);
		execlp("xarchiver", "xarchiver", putanja, NULL);
		execlp("xdg-open", "xdg-open", putanja, NULL);
		_exit(127);
	}
}
static void listaj_sadrzaj_arhive(WINDOW *win, const char *putanja){
	werase(win);
	box(win, 0, 0);
	mvwprintw(win, 1, 2, _("Listing contents of: %s"), putanja);
	wrefresh(win);
	char cmd[PATH_MAX + 64];
	if (g_str_has_suffix(putanja, ".zip")){
		snprintf(cmd, sizeof(cmd), "unzip -l \"%s\"", putanja);
	}
	else if (g_str_has_suffix(putanja, ".rar")){
		snprintf(cmd, sizeof(cmd), "unrar l \"%s\"", putanja);
	}
	else{
		snprintf(cmd, sizeof(cmd), "tar -tf \"%s\"", putanja);
	}
	FILE *fp = popen(cmd, "r");
	if (!fp){
		return;
	}
	char line[512];
	int y = 3;
	int h;
	getmaxyx(win, h, (int){0});
	while (fgets(line, sizeof(line), fp) && y < h - 1){
		line[strcspn(line, "\n")] = 0;
		mvwprintw(win, y++, 2, "%s", line);
	}
	pclose(fp);
	mvwprintw(win, h - 2, 2, _("Press any key to close"));
	wrefresh(win);
	wgetch(win);
}
static int popup_arhiva_meni(WINDOW *parent, const char *ime){
	int ph, pw;
	getmaxyx(parent, ph, pw);
	const char *items[] ={
		_("1. Extract here"),
		_("2. Extract to..."),
		_("3. Open in archive manager"),
		_("4. List contents"),
		_("5. Cancel")
	};
	int fh = 9;
	int fw = pw - 2;
	if (fw < 20){
		fw = 20;
	}
	int sy = (ph - fh) / 2;
	int sx = 1;
	WINDOW *popup = newwin(fh, fw, sy, sx);
	wbkgd(popup, COLOR_PAIR(11));
	box(popup, 0, 0);
	char header[PATH_MAX + 32];
	snprintf(header, sizeof(header), _("Archive: %s"), ime);
	header[fw - 4] = '\0';
	mvwprintw(popup, 1, 2, "%s", header);
	for (int i = 0; i < 5; i++){
		mvwprintw(popup, 3 + i, 4, "%s", items[i]);
	}
	wrefresh(popup);
	int c = wgetch(popup);
	delwin(popup);
	touchwin(parent);
	wrefresh(parent);
	return c;
}
static void extract_here(const char *putanja){
	int ph, pw;
	getmaxyx(stdscr, ph, pw);
	int h = 20;
	if (h > ph - 2) h = ph - 2;
	int w = pw - 2;
	int y = (ph - h) / 2;
	int x = 1;
	WINDOW *win = newwin(h, w, y, x);
	wbkgd(win, COLOR_PAIR(11));
	box(win, 0, 0);
	char header[PATH_MAX + 64];
	snprintf(header, sizeof(header), _("Extracting: %s"), putanja);
	header[w - 4] = '\0';
	mvwprintw(win, 0, 2, "%s", header);
	wrefresh(win);
	char real_archive[PATH_MAX];
	if (!realpath(putanja, real_archive)){
		mvwprintw(win, 2, 2, _("ERROR: realpath() failed"));
		wrefresh(win);
		wgetch(win);
		delwin(win);
		return;
	}
	char dirbuf[PATH_MAX];
	strncpy(dirbuf, real_archive, sizeof(dirbuf) - 1);
	dirbuf[sizeof(dirbuf) - 1] = '\0';
	char *dest_dir = dirname(dirbuf);
	char cmd[PATH_MAX * 2 + 128];
	snprintf(cmd, sizeof(cmd), "unzip -o \"%s\" -d \"%s\" 2>&1", real_archive, dest_dir);
	FILE *fp = popen(cmd, "r");
	if (!fp){
		mvwprintw(win, 2, 2, _("ERROR: popen() failed"));
		wrefresh(win);
		wgetch(win);
		delwin(win);
		return;
	}
	char line[512];
	int row = 2;
	while (fgets(line, sizeof(line), fp)){
		line[strcspn(line, "\n")] = '\0';
		if (row < h - 2){
			mvwprintw(win, row, 2, "%.*s", w - 4, line);
			wrefresh(win);
			row++;
		}
	}
	pclose(fp);
	mvwhline(win, h - 3, 1, ' ', w - 2);
	mvwprintw(win, h - 2, 2, _("Done! Press any key..."));
	wrefresh(win);
	wgetch(win);
	delwin(win);
	flushinp();
	touchwin(stdscr);
	refresh();
}
static void expand_tilde(const char *in, char *out, size_t out_size){
	if (in[0] == '~'){
		const char *home = g_get_home_dir();
		snprintf(out, out_size, "%s%s", home, in + 1);
	} else{
		strncpy(out, in, out_size);
		out[out_size - 1] = '\0';
	}
}
static void extract_to(const char *putanja, const char *dest){
	int ph, pw;
	getmaxyx(stdscr, ph, pw);
	int h = 20;
	if (h > ph - 2) h = ph - 2;
	int w = pw - 2;
	int y = (ph - h) / 2;
	int x = 1;
	WINDOW *win = newwin(h, w, y, x);
	wbkgd(win, COLOR_PAIR(11));
	box(win, 0, 0);
	char hdr1[PATH_MAX + 64];
	char hdr2[PATH_MAX + 64];
	snprintf(hdr1, sizeof(hdr1), _("Extracting to: %s"), dest);
	snprintf(hdr2, sizeof(hdr2), _("Archive: %s"), putanja);
	hdr1[w - 4] = '\0';
	hdr2[w - 4] = '\0';
	mvwprintw(win, 0, 2, "%s", hdr1);
	mvwprintw(win, 1, 2, "%s", hdr2);
	wrefresh(win);
	char real_archive[PATH_MAX];
	if (!realpath(putanja, real_archive)){
		mvwprintw(win, 3, 2, _("ERROR: realpath() failed"));
		wrefresh(win);
		wgetch(win);
		delwin(win);
		return;
	}
	char real_dest[PATH_MAX];
	expand_tilde(dest, real_dest, sizeof(real_dest));
	if (real_dest[0] == '\0'){
		mvwprintw(win, 3, 2, _("ERROR: invalid destination path"));
		wrefresh(win);
		wgetch(win);
		delwin(win);
		return;
	}
	char cmd[PATH_MAX * 2 + 128];
	snprintf(cmd, sizeof(cmd), "unzip -o \"%s\" -d \"%s\" 2>&1", real_archive, real_dest);
	FILE *fp = popen(cmd, "r");
	if (!fp){
		mvwprintw(win, 3, 2, _("ERROR: popen() failed"));
		wrefresh(win);
		wgetch(win);
		delwin(win);
		return;
	}
	char line[512];
	int row = 3;
	while (fgets(line, sizeof(line), fp)){
		line[strcspn(line, "\n")] = '\0';
		if (row < h - 2){
			mvwprintw(win, row, 2, "%.*s", w - 4, line);
			wrefresh(win);
			row++;
		}
	}
	pclose(fp);
	mvwhline(win, h - 3, 1, ' ', w - 2);
	mvwprintw(win, h - 2, 2, _("Done! Press any key..."));
	wrefresh(win);
	wgetch(win);
	delwin(win);
	wtimeout(stdscr, -1);
	nodelay(stdscr, FALSE);
	keypad(stdscr, TRUE);
	noecho();
	cbreak();
	curs_set(0);
	flushinp();
	touchwin(stdscr);
	refresh();
}
static gboolean je_zabranjena_ext(const char *ime){
	const char *ext = strrchr(ime, '.');
	if (!ext) return FALSE;
	const char *zabranjene[] ={
		".A26","A78",".exe",
		".bin",".cue",".iso",".nes",".bsz",".srm",".oops",
		".cab",".dll",".nfo",".ico",
		".ods",".fbl",".fda",".fsp",".fpa",".hnr",
		".poi",".docm","pdfdf",".md5",".nrg",
		".sit",
		".AppImage", NULL
	};
	for (int i = 0; zabranjene[i]; i++){
		if (g_ascii_strcasecmp(ext, zabranjene[i]) == 0)
			return TRUE;
	}
	return FALSE;
}
static void fm_ucitaj(FileManager *fm){
	if (!fm){
		return;
	}
	fm_cpu_ciscenje = FALSE;

	fm_bookmarks_load();
	if (fm->stavke){
		g_ptr_array_free(fm->stavke, TRUE);
	}
	fm->stavke = g_ptr_array_new_with_free_func(g_free);
	GPtrArray *dirs  = g_ptr_array_new_with_free_func(g_free);
	GPtrArray *files = g_ptr_array_new_with_free_func(g_free);

	DIR *d = opendir(fm->trenutni_put);
	if (!d){
		g_ptr_array_free(dirs, TRUE);
		g_ptr_array_free(files, TRUE);
		return;
	}
	struct dirent *ent;
	while ((ent = readdir(d))){
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0){
			continue;
		}
		if (!fm_prikazi_skrivene && ent->d_name[0] == '.'){
			continue;
		}
		char *puni_put = g_build_filename(fm->trenutni_put, ent->d_name, NULL);
		struct stat st;
		if (stat(puni_put, &st) == 0 && S_ISDIR(st.st_mode)){
			g_ptr_array_add(dirs, g_strdup(ent->d_name));
		} else{
			g_ptr_array_add(files, g_strdup(ent->d_name));
		}
		g_free(puni_put);
	}
	closedir(d);
	g_ptr_array_sort(dirs, (GCompareFunc)poredjaj_foldere_abecedno);
	g_ptr_array_sort(files, (GCompareFunc)poredjaj_fajlove_abecedno);
	for (guint i = 0; i < dirs->len; i++){
		g_ptr_array_add(fm->stavke, g_strdup(g_ptr_array_index(dirs, i)));
	}
	for (guint i = 0; i < files->len; i++){
		g_ptr_array_add(fm->stavke, g_strdup(g_ptr_array_index(files, i)));
	}
	g_ptr_array_free(dirs, TRUE);
	g_ptr_array_free(files, TRUE);
	fm->selektovan = 0;
	fm->offset = 0;
}
static void fm_crtaj(WINDOW *win, FileManager *fm){
	werase(win);
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	box(win, 0, 0);
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	int h, w;
	getmaxyx(win, h, w);
	mvwprintw(win, 1, 2, _("File Manager"));
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	mvwhline(win, 2, 1, ACS_HLINE, w - 2);
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	mvwprintw(win, 3, 2, "%s", fm->trenutni_put);
	int list_start = 5;
	int footer_height = 1;
	int max_prikaz = h - list_start - footer_height - 1;
	for (int i = 0; i < max_prikaz; i++){
		int idx = fm->offset + i;
		if (idx >= (int)fm->stavke->len){
			break;
		}
		char *ime = g_ptr_array_index(fm->stavke, idx);
		char *puni_put = g_build_filename(fm->trenutni_put, ime, NULL);
		struct stat st;
		int is_dir = (stat(puni_put, &st) == 0 && S_ISDIR(st.st_mode));
		g_free(puni_put);
		int y = list_start + i;
		gboolean marked = FALSE;
		if (y >= h - footer_height){ 
			break;
		}
		if (fm->selektovani){
			for (guint j = 0; j < fm->selektovani->len; j++){
				if (g_strcmp0(ime, g_ptr_array_index(fm->selektovani, j)) == 0){
					marked = TRUE;
					break;
				}
			}
		}
		if (marked){
			wattron(win, COLOR_PAIR(PLAVA_BOJA) | A_BOLD);
		}
		if (idx == fm->selektovan){
			wattron(win, A_REVERSE);
			mvwprintw(win, y, 1, ">");
		} else{
			mvwprintw(win, y, 1, " ");
		}
		if (is_dir){
			mvwprintw(win, y, 3, "[DIR] %s", ime);
		} else{
    			const char *ext = strrchr(ime, '.');
			if (ext && g_ascii_strcasecmp(ext, ".m3u") == 0){
				if (!marked){
					wattron(win, COLOR_PAIR(PLAVA_BOJA) | A_BOLD);
				}
				mvwprintw(win, y, 3, "      %s", ime);
				if (!marked){
					wattroff(win, COLOR_PAIR(PLAVA_BOJA) | A_BOLD);
				}
			} else if (ext && g_ascii_strcasecmp(ext, ".mp3") == 0){
				if (!marked){
					wattron(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
				mvwprintw(win, y, 3, "      %s", ime);
				if (!marked){
					wattroff(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
			} else if (ext && g_ascii_strcasecmp(ext, ".flac") == 0){
				if (!marked){
					wattron(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
				mvwprintw(win, y, 3, "      %s", ime);
				if (!marked){
					wattroff(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
			} else if (ext && g_ascii_strcasecmp(ext, ".wav") == 0){
				if (!marked){
					wattron(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
				mvwprintw(win, y, 3, "      %s", ime);
				if (!marked){
					wattroff(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
			} else if (ext && g_ascii_strcasecmp(ext, ".ogg") == 0){
				if (!marked){
					wattron(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
				mvwprintw(win, y, 3, "      %s", ime);
				if (!marked){
					wattroff(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
			} else if (ext && g_ascii_strcasecmp(ext, ".m4a") == 0){
				if (!marked){
					wattron(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
				mvwprintw(win, y, 3, "      %s", ime);
				if (!marked){
					wattroff(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
			} else if (ext && g_ascii_strcasecmp(ext, ".aac") == 0){
				if (!marked){
					wattron(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
				mvwprintw(win, y, 3, "      %s", ime);
				if (!marked){
					wattroff(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
			} else if (ext && g_ascii_strcasecmp(ext, ".opus") == 0){
				if (!marked){
					wattron(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
				mvwprintw(win, y, 3, "      %s", ime);
				if (!marked){
					wattroff(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
				}
			} else{
				if (!marked){
					wattron(win, COLOR_PAIR(SIVA_BOJA) | A_BOLD);
				}
				mvwprintw(win, y, 3, "      %s", ime);
				if (!marked){
					wattroff(win, COLOR_PAIR(SIVA_BOJA) | A_BOLD);
				}
			}
		}
		if (marked){
			wattroff(win, COLOR_PAIR(PLAVA_BOJA) | A_BOLD);
		}
		if (idx == fm->selektovan){
			wattroff(win, A_REVERSE);
		}
	}
	if (footer_privremeni){
		footer_poruka(win);
	} else{
		footer_datoteka(win);
	}
	wrefresh(win);
	if (fm_cpu_ciscenje){
		fm_cpu_ciscenje = FALSE;
	}
}
static int fm_key(FileManager *fm, int ch){
	provera_zavrsetka_pesme();
	footer_osvezavanje(stdscr);
	int h, w;
	getmaxyx(stdscr, h, w);
	(void)w;
	switch (ch){
		case KEY_UP:
			if (fm->selektovan > 0){
				fm->selektovan--;
			}
			if (fm->selektovan < fm->offset){
				fm->offset = fm->selektovan;
			}
			break;

		case KEY_DOWN:
			int list_start = 5;
			int max_prikaz = h - list_start - 3;
			if (fm->selektovan + 1 < (int)fm->stavke->len){
				fm->selektovan++;
			}
			if (fm->selektovan >= fm->offset + max_prikaz){
				fm->offset = fm->selektovan - max_prikaz + 1;
			}
			break;

		case 10:{ // ENTER
			char *ime = g_ptr_array_index(fm->stavke, fm->selektovan);
			char *novi_put = g_build_filename(fm->trenutni_put, ime, NULL);
			struct stat st;
			if (stat(novi_put, &st) == 0 && S_ISDIR(st.st_mode)){
				realpath(novi_put, fm->trenutni_put);
				fm_ucitaj(fm);
			} else{
				const char *ext = strrchr(ime, '.');
				if (ext && g_ascii_strcasecmp(ext, ".m3u") == 0){
					if (radio_prozor){
						guint ucitano = ucitavanje_m3u_liste_radio(novi_put);
						if(ucitano > 0){
							radio.trenutna = 0;
							sacuvaj_m3u_liste_radio();
							footer_prikaz_poruka(stdscr, _(" %s playlist is loaded "), ime);
							fm_cpu_ciscenje = TRUE;
						} else{
							footer_prikaz_poruka(stdscr, _(" %s Playlist not loaded, no radio streams found "), ime);
						}
					} else{
						guint ucitano = ucitavanje_m3u_liste(&playlist, novi_put);
                				if (ucitano > 0){
							cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
							footer_prikaz_poruka(stdscr, _(" %s playlist is loaded "), ime);
							fm_cpu_ciscenje = TRUE;
						} else{
							footer_prikaz_poruka(stdscr, _(" %s Playlist not loaded, contains URL stream "), ime);
						}
					}
					g_free(novi_put);
					break;
				} else if (ext && g_ascii_strcasecmp(ext, ".mp3") == 0){
					if (radio_prozor){
						footer_prikaz_poruka(stdscr, _(" %s File not loaded, no radio streams found "), ime);
					} else{
						dodavanje_fajla_u_listu(&playlist, novi_put);
						cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
						footer_prikaz_poruka(stdscr, _(" %s song has been added to the playlist "), ime);
						fm_cpu_ciscenje = TRUE;
					}
				} else if (ext && g_ascii_strcasecmp(ext, ".wav") == 0){
					if (radio_prozor){
						footer_prikaz_poruka(stdscr, _(" %s File not loaded, no radio streams found "), ime);
					} else{
						dodavanje_fajla_u_listu(&playlist, novi_put);
						cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
						footer_prikaz_poruka(stdscr, _(" %s song has been added to the playlist "), ime);
						fm_cpu_ciscenje = TRUE;
					}
				} else if (ext && g_ascii_strcasecmp(ext, ".flac") == 0){
					if (radio_prozor){
						footer_prikaz_poruka(stdscr, _("  %s File not loaded, no radio streams found "), ime);
					} else{
						dodavanje_fajla_u_listu(&playlist, novi_put);
						cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
						footer_prikaz_poruka(stdscr, _(" %s song has been added to the playlist "), ime);
						fm_cpu_ciscenje = TRUE;
					}
				} else if (ext && g_ascii_strcasecmp(ext, ".ogg") == 0){
					if (radio_prozor){
						footer_prikaz_poruka(stdscr, _(" %s File not loaded, no radio streams found "), ime);
					} else{
						dodavanje_fajla_u_listu(&playlist, novi_put);
						cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
						footer_prikaz_poruka(stdscr, _(" %s song has been added to the playlist "), ime);
						fm_cpu_ciscenje = TRUE;
					}
				} else if (ext && g_ascii_strcasecmp(ext, ".m4a") == 0){
					if (radio_prozor){
						footer_prikaz_poruka(stdscr, _(" %s File not loaded, no radio streams found "), ime);
					} else{
						dodavanje_fajla_u_listu(&playlist, novi_put);
						cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
						footer_prikaz_poruka(stdscr, _(" %s song has been added to the playlist "), ime);
						fm_cpu_ciscenje = TRUE;
					}
				} else if (ext && g_ascii_strcasecmp(ext, ".aac") == 0){
					if (radio_prozor){
						footer_prikaz_poruka(stdscr, _(" %s File not loaded, no radio streams found "), ime);
					} else{
						dodavanje_fajla_u_listu(&playlist, novi_put);
						cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
						footer_prikaz_poruka(stdscr, _(" %s song has been added to the playlist "), ime);
						fm_cpu_ciscenje = TRUE;
					}
				} else if (ext && g_ascii_strcasecmp(ext, ".opus") == 0){
					if (radio_prozor){
						footer_prikaz_poruka(stdscr, _(" %s File not loaded, no radio streams found "), ime);
					} else{
						dodavanje_fajla_u_listu(&playlist, novi_put);
						cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
						footer_prikaz_poruka(stdscr, _(" %s song has been added to the playlist "), ime);
						fm_cpu_ciscenje = TRUE;
					}
				} else if (je_arhiva(ime)){
					WINDOW *win = stdscr;
					int izbor = popup_arhiva_meni(win, ime);
					if (izbor == '1' || izbor == 'e'){
						extract_here(novi_put);
						fm_ucitaj(fm);
					} else if (izbor == '2' || izbor == 't'){
						char dest[PATH_MAX];
						if (zahtev_unosa_esc(win, _("Extract to: "), dest, sizeof(dest))){
							extract_to(novi_put, dest);
							fm_ucitaj(fm);
						}
					} else if (izbor == '3' || izbor == 'o'){
						endwin();
						otvori_u_arhiv_menedzeru(novi_put);
						refresh();
						initscr();
						noecho();
						curs_set(0);
						keypad(stdscr, TRUE);
					} else if (izbor == '4' || izbor == 'l'){
						listaj_sadrzaj_arhive(win, novi_put);
					}
					fm_cpu_ciscenje = TRUE;
					g_free(novi_put);
 					break;
				} else if (je_image_fajl(ime)){
					endwin();
					otvori_image(novi_put);
					refresh();
					initscr();
					noecho();
					curs_set(0);
					keypad(stdscr, TRUE);
					fm_cpu_ciscenje = TRUE;
					g_free(novi_put);
					break;
				} else if (je_video_fajl(ime)){
					endwin();
					otvori_video(novi_put);
					initscr();
					noecho();
					curs_set(0);
					keypad(stdscr, TRUE);
					fm_cpu_ciscenje = TRUE;
					g_free(novi_put);
					break;
				} else if (je_office_fajl(ime)){
					endwin();
					otvori_office(novi_put);
					initscr();
					noecho();
					curs_set(0);
					keypad(stdscr, TRUE);
					fm_cpu_ciscenje = TRUE;
					g_free(novi_put);
					break;
				} else{
					if (!je_zabranjena_ext(ime)){
						const char *editor = nadji_editor();
						if (!editor){
							footer_prikaz_poruka(stdscr, _(" No editor found (nano/vim) "));
							return 1;
						}
						endwin();
						char cmd[PATH_MAX + 64];
						snprintf(cmd, sizeof(cmd), "%s \"%s\"", editor, novi_put);
						system(cmd);
						refresh();
						initscr();
						noecho();
						curs_set(0);
						keypad(stdscr, TRUE);
					} else{
						footer_prikaz_poruka(stdscr, _(" %s is not editable "), ime);
					}
					fm_cpu_ciscenje = TRUE;
				}
			}
			g_free(novi_put);
			break;
		}
		case KEY_BACKSPACE:
		case 127:
		case KEY_LEFT:{
			char *parent = g_path_get_dirname(fm->trenutni_put);
			if (parent){
				strncpy(fm->trenutni_put, parent, sizeof(fm->trenutni_put) - 1);
				fm->trenutni_put[sizeof(fm->trenutni_put) - 1] = '\0';
				g_free(parent);
				fm_ucitaj(fm);
			}
			break;
		}
		//case 'k': //  k - zatvori program
		case 27:  //  ESC - zatvori program
			fm_cpu_ciscenje = TRUE;
			return -1; 
		case 'f':{ // ubaci sve podatke iz foldera
			char *ime = g_ptr_array_index(fm->stavke, fm->selektovan);
			char *novi_put = g_build_filename(fm->trenutni_put, ime, NULL);
			if (!folder_ima_audio_fajlove(novi_put)){
				footer_prikaz_poruka(stdscr, _(" Folder \"%s\" does not contain an audio file "), ime);
				g_free(novi_put);
				fm_cpu_ciscenje = TRUE;
				break;
			}
			dodavanje_celog_foldera(&playlist, novi_put);
			cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
			footer_prikaz_poruka(stdscr, _(" Folder \"%s\" is loaded "), ime);
			g_free(novi_put);
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 'F':{ // Ubaci podatke iz foldera
			if (!folder_ima_audio_fajlove(fm->trenutni_put)){
				footer_prikaz_poruka(stdscr, _(" Folder \"%s\" does not contain an audio file "), fm->trenutni_put);
				fm_cpu_ciscenje = TRUE;
				break;
			}
			dodavanje_celog_foldera(&playlist, fm->trenutni_put);
			cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
			footer_prikaz_poruka(stdscr, _(" Folder \"%s\" is loaded "), fm->trenutni_put);
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 'n':{ //ctrl+n napravi novu fasciklu
			char ime_nove_fascikle[256];
			int h, y, x;
			getmaxyx(stdscr, h, w);
			echo();
			curs_set(1);
			const char *prompt = _(" Enter new folder name: ");
			move(h - 1, 0);
			clrtoeol();
			mvprintw(h - 1, 1, "%s", prompt);
			getyx(stdscr, y, x);
			mvwgetnstr(stdscr, y, x, ime_nove_fascikle, sizeof(ime_nove_fascikle) - 1);
			noecho();
			curs_set(0);
			if (strlen(ime_nove_fascikle) > 0){
				char *puni_put = g_build_filename(fm->trenutni_put, ime_nove_fascikle, NULL);
				if (mkdir(puni_put, 0755) == 0){
					footer_prikaz_poruka(stdscr, _(" Folder \"%s\" created "), ime_nove_fascikle);
					fm_ucitaj(fm);
				} else{
					footer_prikaz_poruka(stdscr, _(" Failed to create folder \"%s\" "), ime_nove_fascikle);
				}
				g_free(puni_put);
			}
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 't':{ //ctrl+t naprvi novi dokument
			char ime_nove_datoteke[256];
			int h, y, x;
			getmaxyx(stdscr, h, w);
			echo();
			curs_set(1);
			const char *prompt = _(" Enter new text file name: ");
			move(h - 1, 0);
			clrtoeol();
			mvprintw(h - 1, 1, "%s", prompt);
			getyx(stdscr, y, x);
			mvwgetnstr(stdscr, y, x, ime_nove_datoteke, sizeof(ime_nove_datoteke) - 1);
			noecho();
			curs_set(0);
			if (strlen(ime_nove_datoteke) > 0){
				char *puni_put = g_build_filename(fm->trenutni_put, ime_nove_datoteke, NULL);
				FILE *f = fopen(puni_put, "w");
				if (f){
					fclose(f);
					footer_prikaz_poruka(stdscr, _(" File \"%s\" created "), ime_nove_datoteke);
					fm_ucitaj(fm);
				} else{
					footer_prikaz_poruka(stdscr, _(" Failed to create file \"%s\" "), ime_nove_datoteke);
				}
				g_free(puni_put);
			}
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case KEY_DC:{ //delete
			GPtrArray *targets = (fm->selektovani && fm->selektovani->len > 0)
			? fm->selektovani
			: NULL;
			if (!targets){
				targets = g_ptr_array_new_with_free_func(g_free);
				g_ptr_array_add(targets, g_strdup(g_ptr_array_index(fm->stavke, fm->selektovan)));
			}
			gboolean multi = (targets->len > 1);
			for (guint t = 0; t < targets->len; t++){
				char *ime = g_ptr_array_index(targets, t);
				char *puni_put = g_build_filename(fm->trenutni_put, ime, NULL);
				footer_prikaz_poruka(stdscr, _(" Delete \"%s\" ? (y/n) "), ime);
				int potvrda = getch();
				if (potvrda != 'y' && potvrda != 'Y'){
					g_free(puni_put);
					continue;
				}
				if (!multi){
					// Kreiraj undo folder
					g_mkdir_with_parents("/tmp/fm_undo", 0700);
					g_free(fm->undo_path_src);
					g_free(fm->undo_path_dest);
					fm->undo_path_src = g_build_filename("/tmp/fm_undo", ime, NULL);
					fm->undo_path_dest = g_strdup(fm->trenutni_put);
					fm->last_action = ACTION_DELETE;
					// Kopiraj fajl/folder u /tmp/fm_undo
					if (g_file_test(puni_put, G_FILE_TEST_IS_DIR)){
						gchar *cmd_cp = g_strdup_printf("cp -r \"%s\" \"%s\"", puni_put, fm->undo_path_src);
						system(cmd_cp);
						g_free(cmd_cp);
						gchar *cmd_rm = g_strdup_printf("rm -rf \"%s\"", puni_put);
						system(cmd_rm);
						g_free(cmd_rm);
					} else{
						gchar *cmd_cp = g_strdup_printf("cp \"%s\" \"%s\"", puni_put, fm->undo_path_src);
						system(cmd_cp);
						g_free(cmd_cp);
						remove(puni_put);
					}
					footer_prikaz_poruka(stdscr, _(" Deleted \"%s\" (can undo) "), ime);
				} else{
					// Multi delete → undo onemogućen
					fm->last_action = ACTION_NONE;
					gchar *cmd = g_strdup_printf("rm -rf \"%s\"", puni_put);
					system(cmd);
					g_free(cmd);
				}
				g_free(puni_put);
			}
			if (multi){
				footer_prikaz_poruka(stdscr, _(" Deleted %d items (undo disabled )"), targets->len);
			}
			if (targets != fm->selektovani){
				g_ptr_array_free(targets, TRUE);
			}
			if (fm->selektovani){
				g_ptr_array_set_size(fm->selektovani, 0);
			}
			fm_ucitaj(fm);
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 's':{ //s pretrazi
			char query[256];
			int h, x;
			int ch, i = 0;
			getmaxyx(stdscr, h, w);
			echo();
			curs_set(1);
			const char *prompt = _(" Search file or folder: ");
			move(h - 1, 0);
			clrtoeol();
			mvprintw(h - 1, 1, "%s", prompt);
			getyx(stdscr, (int){0}, x);
			memset(query, 0, sizeof(query));
			memset(query, 0, sizeof(query));
			while (1){
				ch = mvgetch(h - 1, x + i);
				if (ch == 27){ // ESC
					query[0] = '\0';
					break;
				} else if (ch == '\n'){ // Enter
					break;
				} else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && i > 0){
					i--;
					query[i] = '\0';
					mvaddch(h - 1, x + i, ' ');
					move(h - 1, x + i);
				} else if (isprint(ch) && i < (int)sizeof(query) - 1){
					query[i++] = ch;
					mvaddch(h - 1, x + i - 1, ch);
				}
			}
			noecho();
			curs_set(0);
			// ESC ili prazno → vrati punu listu
			if (query[0] == '\0'){
				if (fm->backup_stavke){
					g_ptr_array_free(fm->stavke, TRUE);
					fm->stavke = fm->backup_stavke;
					fm->backup_stavke = NULL;
					fm->selektovan = 0;
					fm->offset = 0;
				}
				fm_cpu_ciscenje = TRUE;
				break;
			}
			// Ako backup ne postoji ili je folder promenjen → napravi novi backup
			if (!fm->backup_stavke){
				fm->backup_stavke = g_ptr_array_new_with_free_func(g_free);
				for (guint j = 0; j < fm->stavke->len; j++){
					g_ptr_array_add(fm->backup_stavke, g_strdup(g_ptr_array_index(fm->stavke, j)));
				}
			} else{
				g_ptr_array_free(fm->backup_stavke, TRUE);
				fm->backup_stavke = g_ptr_array_new_with_free_func(g_free);
				for (guint j = 0; j < fm->stavke->len; j++){
					g_ptr_array_add(fm->backup_stavke, g_strdup(g_ptr_array_index(fm->stavke, j)));
				}
			}
			// očisti trenutnu listu stavki pre filtriranja
			g_ptr_array_set_size(fm->stavke, 0);
			gchar *q = g_ascii_strdown(query, -1);
			for (guint i = 0; i < fm->backup_stavke->len; i++){
				char *ime = g_ptr_array_index(fm->backup_stavke, i);
				gchar *low = g_ascii_strdown(ime, -1);
				if (strstr(low, q)){
					g_ptr_array_add(fm->stavke, g_strdup(ime));
				}
				g_free(low);
			}
			g_free(q);
			if (fm->stavke->len == 0){
				footer_prikaz_poruka(stdscr, _(" No match for: %s "), query);
				if (fm->backup_stavke){
					g_ptr_array_free(fm->stavke, TRUE);
					fm->stavke = fm->backup_stavke;
					fm->backup_stavke = NULL;
					fm->selektovan = 0;
					fm->offset = 0;
				}
				fm_cpu_ciscenje = TRUE;
				break;
			}
			fm->selektovan = 0;
			fm->offset = 0;
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 'c': // c kopiraj
		case 'x':{ // x iseci
			if (!fm->selektovani || fm->selektovani->len == 0){
				fm->selektovani = g_ptr_array_new_with_free_func(g_free);
				g_ptr_array_add(fm->selektovani, g_strdup(g_ptr_array_index(fm->stavke, fm->selektovan)));
			}
			if (fm->clipboard){
				g_ptr_array_free(fm->clipboard, TRUE);
			}
			fm->clipboard = g_ptr_array_new_with_free_func(g_free);
			for (guint i = 0; i < fm->selektovani->len; i++){
				char *ime = g_ptr_array_index(fm->selektovani, i);
				char *full_path = g_build_filename(fm->trenutni_put, ime, NULL);
				g_ptr_array_add(fm->clipboard, full_path);
			}
			fm->clipboard_mode = (ch == 'c') ? CLIPBOARD_COPY : CLIPBOARD_CUT;
			footer_prikaz_poruka(stdscr, (ch == 'c') ? _(" Copied %d items ") : _(" Cut %d items "), fm->selektovani->len);
			fm_cpu_ciscenje = TRUE;
			if (fm->selektovani){
				g_ptr_array_free(fm->selektovani, TRUE);
				fm->selektovani = NULL;
			}
    			break;
		}
		case 'v':{ //v - zalepi
			if (!fm->clipboard || fm->clipboard->len == 0){
				footer_prikaz_poruka(stdscr, _(" Clipboard empty "));
				break;
			}
			gboolean multi = (fm->clipboard->len > 1);
			for (guint i = 0; i < fm->clipboard->len; i++){
				char *source = g_ptr_array_index(fm->clipboard, i);
				char *dest = g_build_filename(fm->trenutni_put, g_path_get_basename(source), NULL);
				if (!multi){
					g_free(fm->undo_path_src);
					g_free(fm->undo_path_dest);
					if (fm->clipboard_mode == CLIPBOARD_COPY){
						fm->undo_path_dest = g_strdup(dest);
						fm->last_action = ACTION_COPY;
					} else if (fm->clipboard_mode == CLIPBOARD_CUT){
						fm->undo_path_src = g_strdup(source);
						fm->undo_path_dest = g_strdup(dest);
						fm->last_action = ACTION_CUT;
					}
				} else{
					fm->last_action = ACTION_NONE;
				}
				if (fm->clipboard_mode == CLIPBOARD_COPY){
					if (g_file_test(source, G_FILE_TEST_IS_DIR)){
						gchar *cmd = g_strdup_printf("cp -r \"%s\" \"%s\"", source, dest);
						system(cmd);
						g_free(cmd);
					} else{
						gchar *cmd = g_strdup_printf("cp \"%s\" \"%s\"", source, dest);
						system(cmd);
						g_free(cmd);
					}
				} else if (fm->clipboard_mode == CLIPBOARD_CUT){
					gchar *cmd = g_strdup_printf("mv \"%s\" \"%s\"", source, dest);
					system(cmd);
					g_free(cmd);
				}
				g_free(dest);
			}
			if (multi){
				footer_prikaz_poruka(stdscr, (" Pasted %d items (undo disabled) "), fm->clipboard->len);
			}
			if (fm->clipboard_mode == CLIPBOARD_CUT){
				g_ptr_array_free(fm->clipboard, TRUE);
				fm->clipboard = NULL;
				fm->clipboard_mode = CLIPBOARD_NONE;
			}
			if (fm->selektovani){
				g_ptr_array_set_size(fm->selektovani, 0);
			}
			fm_ucitaj(fm);
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 15:{ //ctrl+o - otvori terminal u trenutnom folderu
			const char *terminal = NULL;
			if (g_file_test("/usr/bin/gnome-terminal", G_FILE_TEST_IS_EXECUTABLE)){
				terminal = "/usr/bin/gnome-terminal";
			} else if (g_file_test("/usr/bin/xfce4-terminal", G_FILE_TEST_IS_EXECUTABLE)){
				terminal = "/usr/bin/xfce4-terminal";
			} else if (g_file_test("/usr/bin/alacritty", G_FILE_TEST_IS_EXECUTABLE)){
				terminal = "/usr/bin/alacritty";
			} else if (g_file_test("/usr/bin/x-terminal-emulator", G_FILE_TEST_IS_EXECUTABLE)){
				terminal = "/usr/bin/x-terminal-emulator";
			}
			if (!terminal){
				footer_prikaz_poruka(stdscr, _(" No supported terminal found! "));
				fm_cpu_ciscenje = TRUE;
				break;
			}
			pid_t pid = fork();
			if (pid == 0){
				// Child process
				if (strstr(terminal, "gnome-terminal") || strstr(terminal, "xfce4-terminal") || strstr(terminal, "alacritty")){
					execlp(terminal, terminal, "--working-directory", fm->trenutni_put, (char *)NULL);
				} else{
					execlp(terminal, terminal, "-e", "bash", "-c", "cd \"$PWD\"; exec bash", (char *)NULL);
				}
				_exit(127);
			} else if (pid > 0){
				int status;
				waitpid(pid, &status, WNOHANG);
			} else{
				footer_prikaz_poruka(stdscr, _(" Failed to fork terminal process "));
			}
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 'u':{ //u - vrati
			if (fm->last_action == ACTION_NONE){
				footer_prikaz_poruka(stdscr, _(" Nothing to undo "));
				fm_cpu_ciscenje = TRUE;
				break;
			}
			switch (fm->last_action){
				case ACTION_COPY:{
					gchar *cmd = g_strdup_printf("rm -rf \"%s\"", fm->undo_path_dest);
					system(cmd);
					g_free(cmd);
					footer_prikaz_poruka(stdscr, _(" Undo copy "));
					break;
				}
				case ACTION_CUT:{
					gchar *cmd = g_strdup_printf("mv \"%s\" \"%s\"", fm->undo_path_dest, fm->undo_path_src);
					system(cmd);
					g_free(cmd);
					footer_prikaz_poruka(stdscr, _(" Undo cut "));
					break;
				}
				case ACTION_DELETE:{
					gchar *cmd = g_strdup_printf("mv \"%s\" \"%s\"", fm->undo_path_src, fm->undo_path_dest);
					system(cmd);
					g_free(cmd);
					footer_prikaz_poruka(stdscr, _(" Undo delete "));
					break;
				}
				default:{
					break;
				}
			}
			fm->last_action = ACTION_NONE;
			fm_ucitaj(fm);
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 'm':{ // m - obelezi fajl/folder
			if (!fm->selektovani){
				fm->selektovani = g_ptr_array_new_with_free_func(g_free);
			}
			char *ime = g_ptr_array_index(fm->stavke, fm->selektovan);
			gboolean found = FALSE;
			for (guint i = 0; i < fm->selektovani->len; i++){
				if (g_strcmp0(ime, g_ptr_array_index(fm->selektovani, i)) == 0){
					found = TRUE;
					g_ptr_array_remove_index(fm->selektovani, i);
					break;
				}
			}
			if (!found){
				g_ptr_array_add(fm->selektovani, g_strdup(ime));
			}
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 1:{ // ctrl+a Selektuj sve stavke
			if (!fm->selektovani){
				fm->selektovani = g_ptr_array_new_with_free_func(g_free);
			} else{
				g_ptr_array_set_size(fm->selektovani, 0);
			}
			for (guint i = 0; i < fm->stavke->len; i++){
				g_ptr_array_add(fm->selektovani, g_strdup(g_ptr_array_index(fm->stavke, i)));
			}
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case KEY_F(2):{ //F2 Rename selektovani folder/fajl
			if (fm->stavke->len == 0){
				break;
			}
			char *old_name = g_ptr_array_index(fm->stavke, fm->selektovan);
			char old_path[PATH_MAX];
			char new_name[256];
			snprintf(old_path, sizeof(old_path), "%.*s/%.*s",
				(int)(sizeof(old_path)-1 - strlen(old_name)-1), fm->trenutni_put,
				(int)(sizeof(old_path)-1 - strlen(fm->trenutni_put)-1), old_name);
			int h;
			getmaxyx(stdscr, h, (int){0});
			echo();
			curs_set(1);
			memset(new_name, 0, sizeof(new_name));
			footer_prikaz_poruka(stdscr, _(" Rename \"%s\" to: "), old_name);
			move(h - 1, 3 + strlen(old_name) + 10);
			wgetnstr(stdscr, new_name, sizeof(new_name) - 1);
			noecho();
			curs_set(0);
			if (new_name[0] == '\0'){
				footer_prikaz_poruka(stdscr, _(" Rename canceled "));
				break;
			}
			char new_path[PATH_MAX];
			snprintf(new_path, sizeof(new_path), "%.*s/%.*s",
				(int)(sizeof(new_path)-1 - strlen(new_name)-1), fm->trenutni_put,
				(int)(sizeof(new_path)-1 - strlen(fm->trenutni_put)-1), new_name);
			if (g_file_test(new_path, G_FILE_TEST_EXISTS)){
				footer_prikaz_poruka(stdscr, _(" File/folder exists. Overwrite? (y/n) "));
				int ch = getch();
				if (ch != 'y' && ch != 'Y'){
					footer_prikaz_poruka(stdscr, _(" Rename canceled "));
					break;
				}
			}
			g_free(fm->undo_path_src);
			fm->undo_path_src = g_strdup(old_path);
			g_free(fm->undo_path_dest);
			fm->undo_path_dest = g_strdup(new_path);
			fm->last_action = ACTION_NONE; 
			if (rename(old_path, new_path) == 0){
				footer_prikaz_poruka(stdscr, _(" Renamed \"%s\" → \"%s\" "), old_name, new_name);
			} else{
				footer_prikaz_poruka(stdscr, _(" Failed to rename \"%s\" "), old_name);
			}
			fm_ucitaj(fm);
			fm->selektovan = 0;
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 49: case 50: case 51://  brojevi od
		case 52: case 53: case 54://     1-9
		case 55: case 56: case 57:{
			int idx = ch - '1';
			if (idx >= 0 && idx < MAX_BOOKMARKS && bookmarks[idx]){
				strncpy(fm->trenutni_put, bookmarks[idx], sizeof(fm->trenutni_put)-1);
				fm->trenutni_put[sizeof(fm->trenutni_put)-1] = '\0';
				fm_ucitaj(fm);
				footer_prikaz_poruka(stdscr, _(" Jumped to bookmark %d "), idx + 1);
			} else{
				footer_prikaz_poruka(stdscr, _(" No bookmark at %d "), idx + 1);
			}
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 2:{ // Ctrl+b → dodaj bookmark
			int i;
			for (i = 0; i < MAX_BOOKMARKS; i++){
				if (!bookmarks[i]){
					bookmarks[i] = g_strdup(fm->trenutni_put);
					fm_bookmarks_save();
					footer_prikaz_poruka(stdscr, _(" Bookmark %d saved "), i + 1);
					break;
				}
			}
			if (i == MAX_BOOKMARKS){
				footer_prikaz_poruka(stdscr, _(" Maximum bookmarks reached "));
			}
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 4:{ // Ctrl+d → delete bookmark
			int idx = fm_prompt_bookmark_index(stdscr);
			if (idx >= 0 && idx < MAX_BOOKMARKS){
				if (bookmarks[idx]){
					g_free(bookmarks[idx]); // ovo obrisati ako koristim fm_bookmarks_delete
					bookmarks[idx] = NULL; // ovo obrisati ako koristim fm_bookmarks_delete
					//fm_bookmark_delete(idx);
					fm_bookmarks_compact(); // ovo obrisati ako koristim fm_bookmarks_delete
					footer_prikaz_poruka(stdscr, _(" Bookmark %d deleted "), idx + 1);
				} else{
					footer_prikaz_poruka(stdscr, _(" No bookmark at %d "), idx + 1);
				}
			}
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 16:{ // Ctrl+P promena chmod-a
			char *ime = g_ptr_array_index(fm->stavke, fm->selektovan);
			char *full_path = g_build_filename(fm->trenutni_put, ime, NULL);
			chmod_ui(full_path);
			g_free(full_path);
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 8:{ // Ctrl+H → toggle hidden files
			fm_prikazi_skrivene = !fm_prikazi_skrivene;
			fm_ucitaj(fm);
			footer_prikaz_poruka(stdscr,fm_prikazi_skrivene ? _(" Hidden files shown ") : _(" Hidden files hidden "));
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case 'd':{ // Dodavanje više selektovanih fajlova u playlistu
			if (!fm->selektovani || fm->selektovani->len == 0){
				footer_prikaz_poruka(stdscr, _(" No files selected "));
				fm_cpu_ciscenje = TRUE;
				break;
			}
			int added = 0;
			for (guint i = 0; i < fm->selektovani->len; i++){
				char *ime = g_ptr_array_index(fm->selektovani, i);
				char *puni_put = g_build_filename(fm->trenutni_put, ime, NULL);
				struct stat st;
				if (stat(puni_put, &st) == 0 && S_ISDIR(st.st_mode)){
					g_free(puni_put);
					continue;
				}
				const char *ext = strrchr(ime, '.');
				if (ext && (
					g_ascii_strcasecmp(ext, ".mp3") == 0 ||
					g_ascii_strcasecmp(ext, ".wav") == 0 ||
					g_ascii_strcasecmp(ext, ".flac") == 0 ||
					g_ascii_strcasecmp(ext, ".ogg") == 0 ||
					g_ascii_strcasecmp(ext, ".m4a") == 0 ||
					g_ascii_strcasecmp(ext, ".aac") == 0 ||
					g_ascii_strcasecmp(ext, ".opus") == 0
				)){
					dodavanje_fajla_u_listu(&playlist, puni_put);
					added++;
				}
				g_free(puni_put);
			}
			if (added > 0){
				cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
				footer_prikaz_poruka(stdscr, _(" %d files added to playlist "), added);
			} else{
				footer_prikaz_poruka(stdscr, _(" No valid audio files selected "));
			}
			fm_cpu_ciscenje = TRUE;
			break;
		}
		case '?':{
			WINDOW *win = stdscr;
			pomoc_komande(win);
			fm_cpu_ciscenje = TRUE;
		}
	}
	return 0;
}
////////////////// RADIO
///////////////////////////
void radio_stanica_free(gpointer data){
	RadioStanica *rs = data;
	if (!rs){
		return;
	}
	g_free(rs->name);
	g_free(rs->url);
	g_free(rs);
}
static void sacuvaj_poslednju_stanicu(){
	if (radio.svira < 0 || radio.svira >= (int)radio.stanice->len){
		return;
	}
	RadioStanica *rs = g_ptr_array_index(radio.stanice, radio.svira);
	if (!rs || !rs->url){
		return;
	}
	FILE *f = fopen(poslednja_pustena_stanica, "w");
	if (!f){
		return;
	}
	fprintf(f, "%s\n", rs->url);
	fclose(f);
}
static void ucitaj_poslednju_stanicu(){
	FILE *f = fopen(poslednja_pustena_stanica, "r");
	if (!f){
		return;
	}
	char buf[1024];
	if (!fgets(buf, sizeof(buf), f)){
		fclose(f);
		return;
	}
	fclose(f);
	g_strstrip(buf);
	for (guint i = 0; i < radio.stanice->len; i++){
		RadioStanica *rs = g_ptr_array_index(radio.stanice, i);
		if (rs && rs->url && strcmp(rs->url, buf) == 0){
			radio.trenutna = i;
			radio.svira = i;
			break;
		}
	}
}
static void sacuvaj_m3u_liste_radio(){
	FILE *f = fopen(radio_fajl, "w");
	if (f){
		for (guint i = 0; i < radio.stanice->len; i++){
			RadioStanica *rs = g_ptr_array_index(radio.stanice, i);
			if (rs && rs->url && *rs->url){
				fprintf(f, "%s|%s\n", rs->name, rs->url);
			}
		}
		fclose(f);
	}
}
static guint ucitavanje_m3u_liste_radio(const char *path){
	if (!path){
		return 0;
	}

	FILE *f = fopen(path, "r");
	if (!f){
		return 0;
	}

	char line[1024];
	char *pending_name = NULL;
	guint count = 0;
	GPtrArray *tmp_stanice = g_ptr_array_new_with_free_func(radio_stanica_free);
	while (fgets(line, sizeof(line), f)){
		str_trim(line);
		if (*line == '\0') continue;

		if (g_str_has_prefix(line, "#EXTINF")){
			char *comma = strchr(line, ',');
			if (comma && *(comma + 1)){
				g_free(pending_name);
				pending_name = g_strdup(comma + 1);
			}
			continue;
		}

		if (*line == '#') continue;
		if (!strstr(line, "://")) continue;
		char *name = pending_name ? pending_name : line;
		char *url  = line;
		RadioStanica *rs = g_malloc0(sizeof(RadioStanica));
		rs->name = g_strdup(name);
		rs->url  = g_strdup(url);
		g_ptr_array_add(tmp_stanice, rs);
		count++;
		g_free(pending_name);
		pending_name = NULL;
	}
	fclose(f);
	if (count > 0){
		if (!radio.stanice){
			radio.stanice = g_ptr_array_new_with_free_func(radio_stanica_free);
		}
		for (guint i = 0; i < tmp_stanice->len; i++){
			RadioStanica *rs = g_ptr_array_index(tmp_stanice, i);
			g_ptr_array_add(radio.stanice, rs);
		}
		tmp_stanice->len = 0;
		g_ptr_array_free(tmp_stanice, TRUE);
	} else{
		g_ptr_array_free(tmp_stanice, TRUE);
	}
	g_free(pending_name);
	return count;
}
static void ucitavanje_tradio_liste_radio(const char *path){
    	if (!path){
		return;
	}
	FILE *f = fopen(path, "r");
	if (!f){
		return;
	}
	if (!radio.stanice){
		radio.stanice = g_ptr_array_new();
	} else{
		g_ptr_array_set_size(radio.stanice, 0);
	}
	char line[1024];
	char *pending_name = NULL;
	while (fgets(line, sizeof(line), f)){
		str_trim(line);
		if (*line == '\0'){
			continue;
		}
		if (g_str_has_prefix(line, "#EXTINF")){
			char *comma = strchr(line, ',');
			if (comma && *(comma + 1)){
				g_free(pending_name);
				pending_name = g_strdup(comma + 1);
			}
			continue;
		}
		if (*line == '#'){
			continue;
		}
		char *name = NULL;
		char *url = NULL;

		char *sep = strchr(line, '|');
		if (sep){
			*sep = '\0';
			name = line;
			url = sep + 1;
		} else{
			url = line;
			name = pending_name ? pending_name : line;
		}
		if (!*url){
			continue;
		}
		RadioStanica *rs = g_malloc0(sizeof(RadioStanica));
		rs->name = g_strdup(name);
		rs->url = g_strdup(url);
		g_ptr_array_add(radio.stanice, rs);
		g_free(pending_name);
		pending_name = NULL;
	}
	fclose(f);
	g_free(pending_name);
	radio.trenutna = 0;
}
static void radio_init(){
	radio.top_index = 0;
	radio.visible_rows = LINES - 12;
	radio.trenutna = 0;
	radio.svira = -1;
	radio.online = 0;
	radio.backup_stanice = NULL;
	radio.volume = 1.0;
	if (radio.stanice){
		g_ptr_array_free(radio.stanice, TRUE);
	}
	radio.stanice = g_ptr_array_new_with_free_func(radio_stanica_free);
	ucitavanje_tradio_liste_radio(radio_fajl);
	ucitaj_poslednju_stanicu();
	if (radio.svira >= 0 && radio.svira < (int)radio.stanice->len){
		radio.trenutna = radio.svira;
	}
	if (radio.trenutna < radio.visible_rows){
		radio.top_index = 0;
	} else{
		radio.top_index = radio.trenutna - radio.visible_rows + 1;
	}
	radio_cpu_ciscenje = 1;
}
static void radio_prikaz(WINDOW *win){
	werase(win);
	glava(win);
	if (!radio.stanice || radio.stanice->len == 0){
		int mid_row = LINES / 2;
		int mid_col = (COLS - 17) / 2;
		wattron(win, A_DIM);
		mvwprintw(win, mid_row, mid_col, _("Playlist is empty"));
		wattroff(win, A_DIM);
		if (footer_privremeni){
			footer_poruka(win);
		} else{
			footer_lista(win);
		}
		wrefresh(win);
		return;
	}
	const char *trenutno_svira_url = NULL;
	if (radio.svira >= 0){
		if (radio.backup_stanice && radio.svira < (int)radio.backup_stanice->len){
			RadioStanica *orig = g_ptr_array_index(radio.backup_stanice, radio.svira);
			if (orig){
				trenutno_svira_url = orig->url;
			}
		} else if (radio.svira < (int)radio.stanice->len){
			RadioStanica *orig = g_ptr_array_index(radio.stanice, radio.svira);
			if (orig){
				trenutno_svira_url = orig->url;
			}
		}
	}
	int start_row = 10;
	int max_rows = radio.visible_rows > 0 ? radio.visible_rows : 5;
	int total = (int)radio.stanice->len;
	if (radio.top_index > total - 1){
		radio.top_index = total > 0 ? total - 1 : 0;
	}
	int end_row = radio.top_index + max_rows;
	if (end_row > total){
		end_row = total;
	}
	for (int i = radio.top_index; i < end_row; i++){
		RadioStanica *rs = g_ptr_array_index(radio.stanice, i);

		int svira_ova = 0;
		if (trenutno_svira_url && rs && rs->url){
			if (strcmp(trenutno_svira_url, rs->url) == 0){
				svira_ova = 1;
			}
		}
		const char *play_mark = "  ";
		if (svira_ova && radio.online){
			if (pauziranje){
				play_mark = utf8_ok ? "⏸ " : "||";
			} else{
				play_mark = utf8_ok ? "▶ " : "> ";
			}
		}
		if (svira_ova && radio.online){
			wattron(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
		} else if (i == radio.trenutna){
			wattron(win, A_REVERSE);
		}
		mvwprintw(win, start_row + (i - radio.top_index), 2, "%s %2d. %s", play_mark, i + 1, rs->name);
		if (svira_ova && radio.online){
			wattroff(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
		} else if (i == radio.trenutna){
			wattroff(win, A_REVERSE);
		}
	}
	if (footer_privremeni){
		footer_poruka(win);
	} else{
		footer_lista(win);
	}
	wrefresh(win);
}
static void jacina_zvuka_radio(gdouble vol){
	if (!radio_bin){
		return;
	}
	if (vol < 0.0){
		vol = 0.0;
	}
	if (vol > 1.0){
		vol = 1.0;
	}
	radio.volume = vol;
	g_object_set(radio_bin, "volume", vol, NULL);
	radio_cpu_ciscenje = 1;
}
static void radio_pustanje(){
	tradio_status = TRADIO_PUSTEN;
	if (!radio.stanice || radio.stanice->len == 0){ 
		return;
	}
	RadioStanica *rs = g_ptr_array_index(radio.stanice, radio.trenutna);
	if (!rs || !rs->url || !*rs->url){ 
		return;
	}
	g_strstrip(rs->url);
	if (reprodukcioni_blok){
		gst_element_set_state(reprodukcioni_blok, GST_STATE_NULL);
		playlist.svira = -1;
		pauziranje = 0;
	}
    	if (!radio_bin){
		radio_bin = gst_element_factory_make("playbin", "radio");
		GstElement *audiosink = gst_element_factory_make("pulsesink", NULL);
		if (audiosink){
			g_object_set(radio_bin, "audio-sink", audiosink, NULL);
		}
	}
	gst_element_set_state(radio_bin, GST_STATE_NULL);
	g_object_set(radio_bin, "uri", rs->url, NULL);
	gst_element_set_state(radio_bin, GST_STATE_PLAYING);
	tradio_elapsed = 0;
	tradio_length = 0;
	tradio_track   = radio.trenutna + 1;
	if (rs && rs->name){
		g_strlcpy(tradio_title,  rs->name, sizeof(tradio_title));
		g_strlcpy(tradio_artist, rs->name, sizeof(tradio_artist));
	} else{
		g_strlcpy(tradio_title,  "Radio", sizeof(tradio_title));
		g_strlcpy(tradio_artist, "Radio", sizeof(tradio_artist));
	}
	g_strlcpy(tradio_album, "Live stream", sizeof(tradio_album));
	radio.svira = radio.trenutna;
	radio.online = 1;
	tradio_status = TRADIO_PUSTEN;
	radio_cpu_ciscenje = 1;
	sacuvaj_poslednju_stanicu();
	sacuvaj_tmuzika_status();
}
static void pauziraj_radio(){
	tradio_status = TRADIO_PAUZIRAN;
	if (!radio_prozor){
		return;
	}
	if (!radio_bin){
		return;
	}
	GstState cur = GST_STATE_NULL;
	gst_element_get_state(radio_bin, &cur, NULL, 0);
	if (cur == GST_STATE_PLAYING){
		gst_element_set_state(radio_bin, GST_STATE_PAUSED);
		pauziranje = 1;
	} 
	else if (cur == GST_STATE_PAUSED){
		gst_element_set_state(radio_bin, GST_STATE_PLAYING);
		pauziranje = 0;
	} else{
		radio_pustanje();
		pauziranje = 0;
	}
	radio_cpu_ciscenje = 1;
}
static void radio_zaustavi(){
	tradio_status = TRADIO_ZAUSTAVLJEN;
	if (radio_bin){
		gst_element_set_state(radio_bin, GST_STATE_NULL);
	}
	radio.online = 0;
	radio.svira = -1;
	radio_cpu_ciscenje = 1;
}
static void radio_dodaj(WINDOW *win){
	char name[256], url[512];
	if (!zahtev_unosa_esc(win, _("Station name: "), name, sizeof(name))){
		return;
	}
	if (!zahtev_unosa_esc(win, _("Station URL: "), url, sizeof(url))){
		return;
	}
	RadioStanica *rs = g_malloc(sizeof(RadioStanica));
	rs->name = g_strdup(name);
	rs->url = g_strdup(url);
	g_ptr_array_add(radio.stanice, rs);
	FILE *f = fopen(radio_fajl, "a");
	if (f){
		fprintf(f, "%s|%s\n", name, url);
		fclose(f);
	}
	radio_cpu_ciscenje = 1;
}
static void radio_obrisi_izabran(){
	if (radio.stanice->len == 0){
		return;
	}
	int svira_pre = radio.svira;
	int obrisi = radio.trenutna;
	g_ptr_array_remove_index(radio.stanice, obrisi);
	FILE *f = fopen(radio_fajl, "w");
	if (f){
		for (int i = 0; i < (int)radio.stanice->len; i++){
			RadioStanica *rs = g_ptr_array_index(radio.stanice, i);
			fprintf(f, "%s|%s\n", rs->name, rs->url);
		}
		fclose(f);
	}
	if (radio.trenutna >= (int)radio.stanice->len){
		radio.trenutna = radio.stanice->len - 1;
	}
	if (radio.online && svira_pre == obrisi){
		radio_zaustavi();
	} else if (radio.online && svira_pre > obrisi){
		radio.svira--;
	}
	radio_cpu_ciscenje = 1;
}
static void radio_obrisi_listu(){
	g_ptr_array_set_size(radio.stanice, 0);
	FILE *f = fopen(radio_fajl, "w");
	if (f){
		fclose(f);
	}
	radio_zaustavi();
	radio_cpu_ciscenje = 1;
}
static void pusti_sledecu_stanicu(){
	if (!radio.stanice || radio.stanice->len == 0){
		return;
	}
	if (radio.trenutna < (int)radio.stanice->len - 1){
		radio.trenutna++;
	} else{
		radio.trenutna = 0;
	}
	if (radio.trenutna >= radio.top_index + radio.visible_rows){
		radio.top_index = radio.trenutna - radio.visible_rows + 1;
	}
	if (radio.trenutna < radio.top_index){
		radio.top_index = radio.trenutna;
	}
	if (!(radio.online && radio.svira == radio.trenutna)){
		radio_pustanje();
	}
	radio_cpu_ciscenje = 1;
}
static void pusti_prethodnu_stanicu(){
	if (!radio.stanice || radio.stanice->len == 0){
		return;
	}
	if (radio.trenutna > 0){
		radio.trenutna--;
	} else{
		radio.trenutna = radio.stanice->len - 1;
	}
	if (radio.trenutna < radio.top_index){
		radio.top_index = radio.trenutna;
	}
	if (radio.trenutna >= radio.top_index + radio.visible_rows){
		radio.top_index = radio.trenutna - radio.visible_rows + 1;
	}
	if (!(radio.online && radio.svira == radio.trenutna)){
		radio_pustanje();
	}
	radio_cpu_ciscenje = 1;
}
static void pusti_prvu_stanicu(){
	if (!radio.stanice || radio.stanice->len == 0){
		return;
	}
	radio.trenutna = 0;
	radio.top_index = 0;
	/*if (!(radio.online && radio.svira == radio.trenutna)){
		radio_pustanje();
	}*/
	radio_cpu_ciscenje = 1;
}
static void pusti_poslednju_stanicu(){
	if (!radio.stanice || radio.stanice->len == 0){
		return;
	}
	radio.trenutna = radio.stanice->len - 1;
	if (radio.trenutna >= radio.visible_rows){
		radio.top_index = radio.trenutna - radio.visible_rows + 1;
	} else{
		radio.top_index = 0;
	}
	/*if (!(radio.online && radio.svira == radio.trenutna)){
		radio_pustanje();
	}*/
	radio_cpu_ciscenje = 1;
}
//////////////////////////////////
//////////////////////////////////
static void print_fit(WINDOW *win, int y, int x, int maxw, const char *txt){
	if (!txt || maxw <= 0){ 
		return;
	}
	int len = strlen(txt);
	if (len <= maxw){
		mvwprintw(win, y, x, "%s", txt);
		return;
	}
	if (maxw <= 3){
		mvwprintw(win, y, x, "%.*s", maxw, txt);
		return;
	}
	char buf[512];
	snprintf(buf, sizeof(buf), "%.*s...", maxw - 3, txt);
	mvwprintw(win, y, x, "%s", buf);
}
static void glava(WINDOW *win){
	int width;
	getmaxyx(win, (int){0}, width);
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	box(win, 0, 0);
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	int inner_w = width - 2;
	int mid_col = inner_w * 2 / 3;
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	for (int r = 1; r <= 6; r++){
		mvwaddch(win, r, mid_col, ACS_VLINE);
	}
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	const char *title = _("tmuzika - terminal music player");
	print_fit(win, 3, 2, mid_col - 2, title);
	mvwprintw(win, 4, 2, _("? = help"));
	if (radio_prozor){
		const char *status;
		status = radio.online ? (pauziranje ? _("LIVE: PAUSE") : _("LIVE: ONLINE")) : _("LIVE: OFFLINE");
		mvwprintw(win, 1, mid_col + 2, _("Volume: %.2f"), radio.volume);
		mvwprintw(win, 2, mid_col + 2, "%s", status);
		time_t sada = time(NULL);
		struct tm *vreme_sada = localtime(&sada);
		if (vreme_sada){
			char vreme_bufer[32];
			char datum_bufer[64];
			strftime(vreme_bufer, sizeof(vreme_bufer), "%H:%M:%S", vreme_sada);
			strftime(datum_bufer, sizeof(datum_bufer), "%d.%m.%Y", vreme_sada);
			mvwprintw(win, 3, mid_col + 2, _("Time: %s"), vreme_bufer);
			mvwprintw(win, 4, mid_col + 2, _("Date: %s"), datum_bufer);
		}
		mvwprintw(win, 5, mid_col + 2, _("Radio stations: %d"), (int)radio.stanice->len);
		wattron(win, COLOR_PAIR(SIVA_BOJA));
		mvwhline(win, 7, 1, ACS_HLINE, inner_w);
		wattroff(win, COLOR_PAIR(SIVA_BOJA));
		mvwprintw(win, 8, 1, _("tmuzika - RADIO"));
		wattron(win, COLOR_PAIR(SIVA_BOJA));
		mvwhline(win, 9, 1, ACS_HLINE, inner_w);
		wattroff(win, COLOR_PAIR(SIVA_BOJA));

	} else{
		const char *status = pustanje ? (pauziranje ? _("PAUSE") : _("PLAY")) : _("STOP");
		mvwprintw(win, 1, mid_col + 2, _("Volume: %.2f"), playlist.volume);
		mvwprintw(win, 2, mid_col + 2, _("Status: %-11s"), status);
		const char *rep = ponovi_pesmu ? _("Repeat: SONG") : ponovi_listu ? _("Repeat: PLAYLIST") : nasumicno_pustanje ? _("Shuffle: ON") : _("Repeat: OFF");
		mvwprintw(win, 3, mid_col + 2, "%s", rep);
		gint64 pos = cached_pos;
		gint64 dur = cached_dur;
		char posbuf[32], durbuf[32];
		prikazivanje_vremena(pos, posbuf, sizeof(posbuf));
		prikazivanje_vremena(dur, durbuf, sizeof(durbuf));
		mvwprintw(win, 4, mid_col + 2, _("Time: %s / %s"), posbuf, durbuf);
		int bar_w = inner_w - mid_col - 3;
		if (bar_w < 10){
			bar_w = 10;
		}
		char bar[256];
		int filled = 0;
		if (pos != GST_CLOCK_TIME_NONE && dur > 0){
			filled = (int)((double)pos / dur * bar_w);
			if (filled > bar_w){
				filled = bar_w;
			}
		}
		for (int i = 0; i < bar_w; i++){
			bar[i] = (i < filled) ? '=' : '-';
		}
		bar[bar_w] = '\0';
		mvwprintw(win, 5, mid_col + 2, "[%s]", bar);
		mvwprintw(win, 6, mid_col + 2, _("Playlist songs: %d"), (int)playlist.stavke->len);
		wattron(win, COLOR_PAIR(SIVA_BOJA));
		mvwhline(win, 7, 1, ACS_HLINE, inner_w);
		wattroff(win, COLOR_PAIR(SIVA_BOJA));
		gchar *kratka_putanja = g_strdup("-");
		if (playlist.stavke->len > 0 && playlist.trenutna >= 0){
			const char *uri = g_ptr_array_index(playlist.stavke, playlist.trenutna);
			gchar *f_ime = gst_uri_get_location(uri);
			if (!f_ime){
				f_ime = g_strdup(uri);
			}
			gchar **delovi = g_strsplit(f_ime, "/", -1);
			int len = 0;
			while (delovi[len]) len++;
			if (len >= 2){
				if (len >= 3){
					g_free(kratka_putanja);
					kratka_putanja = g_strdup_printf("%s/%s", delovi[len-3], delovi[len-2]);
				} else{
					g_free(kratka_putanja);
					kratka_putanja = g_strdup(delovi[len-2]);
				}
			}
			g_strfreev(delovi);
			if (f_ime) g_free(f_ime);
		}
		mvwprintw(win, 8, 2, _("Folder: %s"), kratka_putanja);
		wattron(win, COLOR_PAIR(SIVA_BOJA));
		mvwhline(win, 9, 1, ACS_HLINE, inner_w);
		wattroff(win, COLOR_PAIR(SIVA_BOJA));
		g_free(kratka_putanja);
	}
}
static void izvuci_artist_album_title(const char *uri, char *artist, size_t a_len, char *album, size_t al_len, char *title, size_t t_len){
	artist[0] = album[0] = title[0] = '\0';
	gchar *f_ime = gst_uri_get_location(uri);
	if (!f_ime){
		f_ime = g_strdup(uri);
	}
	gchar *base = g_path_get_basename(f_ime);
	gchar *dot = strrchr(base, '.');
	if (dot){
		*dot = '\0';
	}
	strncpy(title, base, t_len - 1);
	gchar *dir = g_path_get_dirname(f_ime);
	gchar *album_name = g_path_get_basename(dir);
	gchar *dir2 = g_path_get_dirname(dir);
	gchar *artist_name = g_path_get_basename(dir2);
	if (artist_name && artist_name[0]){
		strncpy(artist, artist_name, a_len - 1);
	}
	if (album_name && album_name[0]){
		strncpy(album, album_name, al_len - 1);
	}
	g_free(dir2);
	g_free(dir);
	g_free(base);
	g_free(f_ime);
}
static void prikaz_programa(WINDOW *win){
	footer_osvezavanje(win);
	werase(win);
	int row = 0;
	glava(win);
	max_line_length = 0;
	if (!playlist.stavke || playlist.stavke->len == 0){
		int mid_row = LINES / 2;
		int mid_col = (COLS - 17) / 2;
		wattron(win, A_DIM);
		mvwprintw(win, mid_row, mid_col, _("Playlist is empty"));
		wattroff(win, A_DIM);
		if (footer_privremeni){
			footer_poruka(win);
		} else{
			footer_lista(win);
		}
		wrefresh(win);
		return;
	}
	lista_pocetni_red = 10;
	int show = LINES - lista_pocetni_red - 2;
	if (show < 4){
		show = 4;
	}
	lista_prikazivanje = show;
	int start = playlist.trenutna - show/2;
	if (start < 0){
		start = 0;
	}
	row = lista_pocetni_red;
	for (int i = 0; i < show && (start + i) < (int)playlist.stavke->len; ++i){
		int idx = start + i;
		const char *uri = g_ptr_array_index(playlist.stavke, idx);
		gchar *f_ime = gst_uri_get_location(uri);
		if (!f_ime){
			f_ime = g_strdup(uri);
		}
		char artist[128] ={0}, album[128] ={0}, title[128] ={0};
		char *base = g_path_get_basename(f_ime);
		char *dot = strrchr(base, '.');
		if (dot){
			*dot = '\0';
		}
		izvuci_artist_album_title(uri, artist, sizeof(artist), album, sizeof(album), title, sizeof(title));
		if (f_ime){
			g_free(f_ime);
		}
		const char *trenutno_svira_uri = NULL;
		if (playlist.svira >= 0){
			if (playlist.backup_stavke && playlist.svira < (int)playlist.backup_stavke->len){
				trenutno_svira_uri = g_ptr_array_index(playlist.backup_stavke, playlist.svira);
			} else if (playlist.stavke && playlist.svira < (int)playlist.stavke->len){
				trenutno_svira_uri = g_ptr_array_index(playlist.stavke, playlist.svira);
			}
		}
		const char *play_mark = "  ";
		if (trenutno_svira_uri && strcmp(trenutno_svira_uri, uri) == 0){
			if (pauziranje){
				play_mark = utf8_ok ? "⏸ " : "||";
			} else{
				play_mark = utf8_ok ? "▶ " : "> ";
			}
		}
		if (trenutno_svira_uri && strcmp(trenutno_svira_uri, uri) == 0){
			wattron(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
		} else if (idx == playlist.trenutna){
			wattron(win, A_REVERSE);
		}
		char linija[1024];
		snprintf(linija, sizeof(linija), "%s %2d: %s%s%s%s%s",
			play_mark,
			idx + 1,
			artist[0] ? artist : "",
			artist[0] ? " - " : "",
			album[0] ? album : "",
			album[0] ? " - " : "",
			title[0] ? title : "");
		int maxw = COLS - 4;
		int len = strlen(linija);
		if (len > max_line_length){
			max_line_length = len;
		}
		if (len <= 0){
			horizontal_scroll = 0;
		} else if (horizontal_scroll > len - 1){
			horizontal_scroll = len - 1;
		}
		if (horizontal_scroll < 0){
			horizontal_scroll = 0;
		}
		const char *start_ptr = linija + horizontal_scroll;
		char prikaz[maxw + 1];
		strncpy(prikaz, start_ptr, maxw);
		prikaz[maxw] = '\0';
		mvwprintw(win, row++, 2, "%-*s", maxw, prikaz);
		if (trenutno_svira_uri && strcmp(trenutno_svira_uri, uri) == 0){
			wattroff(win, COLOR_PAIR(ZELENA_BOJA) | A_BOLD);
		} else if (idx == playlist.trenutna){
			wattroff(win, A_REVERSE);
		}
	}
	if (playlist.stavke->len > lista_prikazivanje){
		int scroll_h = lista_prikazivanje;
		int handle_h = scroll_h * lista_prikazivanje / playlist.stavke->len;
		if (handle_h < 1){
			handle_h = 1;
		}
		int start_pos = (playlist.trenutna * scroll_h) / playlist.stavke->len;
		if (start_pos + handle_h > scroll_h){
			start_pos = scroll_h - handle_h;
		}
		int col_scroll = COLS - 2;
		wattron(win, COLOR_PAIR(SIVA_BOJA));
		for (int i = 0; i < scroll_h; i++){
			if (i >= start_pos && i < start_pos + handle_h){
				mvwaddch(win, lista_pocetni_red + i, col_scroll, ACS_CKBOARD);
			} else{
				mvwaddch(win, lista_pocetni_red + i, col_scroll, ACS_VLINE);
			}
		}
		wattroff(win, COLOR_PAIR(SIVA_BOJA));
	}
	if (footer_privremeni){
		footer_poruka(win);
	} else{ 
		footer_lista(win);
	}
	wrefresh(win);
}
static int zahtev_unosa_esc(WINDOW *win, const char *prompt, char *out, size_t outlen){
	wtimeout(win, 100);
	curs_set(1);
	werase(win);
	wattron(win, COLOR_PAIR(SIVA_BOJA));
	box(win, 0, 0);
	wattroff(win, COLOR_PAIR(SIVA_BOJA));
	mvwprintw(win, 1, 1, "%s", prompt);
	footer_unos(win);
	wchar_t wbuf[256];
	mbstowcs(wbuf, prompt, sizeof(wbuf)/sizeof(wchar_t));
	int prompt_len = wcslen(wbuf);
	int x = 1 + prompt_len;
	if (x >= COLS - 2){
		x = COLS - 3;
	}
	wmove(win, 1, x);
	wrefresh(win);
	int pos = 0;
	int ch;
	while (pos < (int)(outlen - 1)){
		provera_zavrsetka_pesme();
		ch = wgetch(win);
		if (ch == 27){  // ESC
			out[0] = '\0';
			curs_set(0);
            		wtimeout(win, 200);
			return 0;
		} else if (ch == '\n' || ch == KEY_ENTER){
			break;
		} else if (ch == KEY_BACKSPACE || ch == 127){
			if (pos > 0){
				pos--;
				wmove(win, 1, x + pos);
				waddch(win, ' ');
				wmove(win, 1, x + pos);
			}
		} else if (ch >= 32 && ch <= 126){
			out[pos++] = (char)ch;
			mvwaddch(win, 1, x + pos - 1, ch);
		}
		wrefresh(win);
	}
	out[pos] = '\0';
	curs_set(0);
	wtimeout(win, 200);
	return 1;
}
static void obrada_poruka(){
	if (radio_prozor){
		return;
	}
	if (!poruke){
		return;
	}
	while (1){
		GstMessage *msg = gst_bus_pop(poruke);
		if (!msg){
			break;
		}
		switch (GST_MESSAGE_TYPE(msg)){
			case GST_MESSAGE_EOS:
    				if (ponovi_pesmu){
        				pusti_izabranu_pesmu();
    				} else if (ponovi_listu){
        				sledeca_pesma();
				} else if (nasumicno_pustanje){
					//nasumicna_pesma();
					sledeca_nasumicna_pesma();
    				} else{
        				if (playlist.trenutna + 1 < (int)playlist.stavke->len){
            					sledeca_pesma();
        				} else{
            					zaustavi_pesmu();
        				}
    				}
    				cpu_ciscenje = 1;
    				break;
			case GST_MESSAGE_ERROR:{
				GError *err = NULL;
				gchar *dbg = NULL;
				gst_message_parse_error(msg, &err, &dbg);
				if (err){
					fprintf(stderr, _(" GStreamer error: %s\n "), err->message);
					g_error_free(err);
				}
				if (dbg){
					g_free(dbg);
				}
				zaustavi_pesmu();
				break;
			}
			default:
				break;
		}
	gst_message_unref(msg);
	}
}
static void pomoc_komande(WINDOW *win){
	int ch;
	wtimeout(win, 200);
	const char *levi_panel[] ={
		_("TMUZIKA"),
		_(" "),
		_("ENTER = play song"),
		_("SPACE = pause / continue"),
		_("b = scroll left, n = scroll right"),
		_("z = stop"),
		_("> = next, < = previous"),
		_("+ = volume up  - = volume down"),
		_("<- = -10s  -> = +10s"),
		_("p = repeat song"),
		_("l = repeat playlist"),
		_("e = shuffle"),
		_("s = search"),
		_("r = Radio mode"),
		_("m = file manager"),
		_("q = save m3u playlist"),
		_("u = load m3u playlist"),
		_("v = go to the current song"),
		_("x = delete song from playlist"),
		_("DELETE = delete all songs from playlist"),
		_("HOME = go to the first song"),
		_("END = go to the last song"),
		_("k = exit")
	};
	int levi_count = sizeof(levi_panel) / sizeof(levi_panel[0]);
	const char *desni_panel[] ={
		_("FILE MANAGER"),
		_(" "),
		_("- ENTER = enter / add song / add .m3u"),
		_("- BACKSPACE = back"),
		_("- ctrl+h = toggle hidden files"),
		_("- f/F = add folder / add current folder"),
		_("- m = select files / folders"),
		_("- ctrl+a = select all"),
		_("- d = add multy files"),
		_("- s = search"),
		_("- n = new folder"),
		_("- t = new file"),
		_("- F2 = rename file / folder"),
		_("- c = copy"),
		_("- x = cut"),
		_("- v = paste"),
		_("- u = undo"),
		_("- ctrl+p = change chmod"),
		_("- ctrl+o = open terminal"),
		_("- DELETE = remove file / folder"),
		_("- ctrl+b = add bookmarks"),
		_("- ctrl+d = remove bookmark"),
		_("- NUMBER 1 - 9 = go to bookmarks"),
		_("- ESC = quit")
	};
	int desni_count = sizeof(desni_panel) / sizeof(desni_panel[0]);
	const char *sredina_radio[] ={
		_("RADIO"),
		_(" "),
		_("ENTER = play radio station"),
		_("SPACE = pause / continue"),
		_("z = stop"),
		_("> = next, < = previous"),
		_("+ = volume up  - = volume down"),
		_("d = add radio station"),
		_("s = search"),
		_("m = file manager"),
		_("q = save m3u playlist"),
		_("u = load radio station m3u playlist"),
		_("v = go to the current radio station"),
		_("x = delete radio station from playlist"),
		_("DELETE = delete all radio station from playlist"),
		_("HOME = go to the first radio station"),
		_("END = go to the last radio station"),
		_("ESC = quit")
	};
	int sredina_count = sizeof(sredina_radio) / sizeof(sredina_radio[0]);
	int offset_panel = 0;
	int offset_radio = 0;
	while (1){
		provera_zavrsetka_pesme();
		werase(win);
		int height, width;
		getmaxyx(win, height, width);
		int panel_height = (height - 3) / 2;
		int max_rows_panel = panel_height;
		int max_rows_radio = height - 2 - panel_height - 1;
		wattron(win, COLOR_PAIR(SIVA_BOJA));
		box(win, 0, 0);
		int inner_w = width - 2;
		int mid_col1 = inner_w * 2 / 4;
		for (int y = 1; y <= panel_height; y++){
			mvwaddch(win, y, mid_col1 + 1, ACS_VLINE);
		}
		wattroff(win, COLOR_PAIR(SIVA_BOJA));
		for (int i = 0; i < max_rows_panel && (i + offset_panel) < levi_count; i++){
			mvwprintw(win, i + 1, 2, "%s", levi_panel[i + offset_panel]);
		}
		for (int i = 0; i < max_rows_panel && (i + offset_panel) < desni_count; i++){
			mvwprintw(win, i + 1, mid_col1 + 3, "%s", desni_panel[i + offset_panel]);
		}
		if (offset_panel > 0){
			mvwaddch(win, 1, width - 2, ACS_UARROW);
		}
		if (offset_panel + max_rows_panel < (levi_count > desni_count ? levi_count : desni_count)){
			mvwaddch(win, panel_height, width - 2, ACS_DARROW);
		}
		int hor_line_row = panel_height + 1;
		for (int x = 1; x < width - 1; x++){
			mvwaddch(win, hor_line_row, x, ACS_HLINE);
		}
		int sredina_start = hor_line_row + 1;
		for (int i = 0; i < max_rows_radio && (i + offset_radio) < sredina_count; i++){
			mvwprintw(win, sredina_start + i, 2, "%s", sredina_radio[i + offset_radio]);
		}
		if (offset_radio > 0){
			mvwaddch(win, sredina_start, width - 2, ACS_UARROW);
		}
		if (offset_radio + max_rows_radio < sredina_count){
			mvwaddch(win, height - 2, width - 2, ACS_DARROW);
		}
		footer_pomoc(win);
		wrefresh(win);
		ch = wgetch(win);
		int max_offset_panel = (levi_count > desni_count ? levi_count : desni_count) - max_rows_panel;
		if (max_offset_panel < 0){
			max_offset_panel = 0;
		}
		int max_offset_radio = sredina_count - max_rows_radio;
		if (max_offset_radio < 0){ 
			max_offset_radio = 0;
		}
		bool focus_radio = (ch == KEY_DOWN || ch == KEY_UP) && getcury(win) > hor_line_row;
		switch (ch){
			case 27: // ESC
				return;
			case KEY_UP:
			case KEY_PPAGE:
				if (focus_radio){
					if (offset_radio > 0){
						offset_radio--;
					}
				} else if (offset_panel > 0){
					offset_panel--;
				}
				break;
			case KEY_DOWN:
			case KEY_NPAGE:
				if (focus_radio){
					if (offset_radio < max_offset_radio){
						offset_radio++;
					}
				} else if (offset_panel < max_offset_panel){
					offset_panel++;
				}
				break;
		}
	}
}
static void cli_pusti_izabranu_pesmu(){
	if (!reprodukcioni_blok || playlist.stavke->len == 0){
		return;
	}
	if (playlist.trenutna < 0){
		playlist.trenutna = 0;
	}
	if (playlist.trenutna >= (int)playlist.stavke->len){
		playlist.trenutna = playlist.stavke->len - 1;
	}
	gst_element_set_state(reprodukcioni_blok, GST_STATE_READY);
	const char *uri = g_ptr_array_index(playlist.stavke, playlist.trenutna);
	g_object_set(reprodukcioni_blok, "uri", uri, NULL);
	playlist.svira = playlist.trenutna;
	gst_element_set_state(reprodukcioni_blok, GST_STATE_PLAYING);
	pustanje = 1;
	pauziranje = 0;
	cpu_ciscenje = 1;
}
static void cli_print_file(const char *filename){
	char path[PATH_MAX];
	snprintf(path, sizeof(path), "%s/%s", TMUZIKA_TMP_DIR, filename);
	FILE *f = fopen(path, "r");
	if(f){
		char buf[512];
		if(fgets(buf, sizeof(buf), f)){
			buf[strcspn(buf, "\n")] = 0; // trim newline
			puts(buf);
		}
		fclose(f);
	}
}
void siguran_exit(int kod){
	zaustavi_pesmu();
	isprazni_playlistu(&playlist);
	if (poruke){
		gst_object_unref(poruke);
		poruke = NULL;
	}
	if (reprodukcioni_blok){
		gst_element_set_state(reprodukcioni_blok, GST_STATE_NULL);
		gst_object_unref(reprodukcioni_blok);
		reprodukcioni_blok = NULL;
	}
	endwin();
	exit(kod);
}
static void cli_pusti_radio(const char *path){
	if (!path || !*path) return;
	trenutni_rezim = REZIM_PLAYER;
	radio_prozor = 1;
	if (radio.stanice){
		g_ptr_array_set_size(radio.stanice, 0);
	} else{
		radio.stanice = g_ptr_array_new_with_free_func(radio_stanica_free);
	}
	if (g_str_has_prefix(path, "http://") || g_str_has_prefix(path, "https://")){
		/* eksplicitno zabranjujemo lokalne fajlove */
		if (strstr(path, ".mp3") || strstr(path, ".wav")){
			fprintf(stderr, _("Error: Local audio files are not allowed with -r\n"));
			siguran_exit(1);
		}
		RadioStanica *rs = g_new0(RadioStanica, 1);
		rs->name = g_strdup(_("CLI Stream"));
		rs->url  = g_strdup(path);
		g_ptr_array_add(radio.stanice, rs);
		radio.trenutna = 0;
		radio_pustanje();
		return;
	}
	struct stat st;
	char tmp_path[PATH_MAX];
	strncpy(tmp_path, path, sizeof(tmp_path) - 1);
	tmp_path[sizeof(tmp_path) - 1] = '\0';
	skracena_putanja_foldera(tmp_path, sizeof(tmp_path));
	if (stat(tmp_path, &st) == 0){
		if (S_ISREG(st.st_mode)){
			const char *ext = strrchr(tmp_path, '.');
			if (ext && g_ascii_strcasecmp(ext, ".m3u") == 0){
				guint count = ucitavanje_m3u_liste_radio(tmp_path);
				if (count == 0){
					fprintf(stderr, _("Error: .m3u file '%s' contains no streams.\n"), tmp_path);
					g_usleep(2 * G_USEC_PER_SEC);
					siguran_exit(1);
				}
			} else{
				fprintf(stderr, _("Error: '%s' is not a stream or .m3u playlist.\n"), tmp_path);
				g_usleep(2 * G_USEC_PER_SEC);
				siguran_exit(1);
			}
		} else{
			fprintf(stderr, _(" Error: '%s' is not a valid file.\n"), tmp_path);
			g_usleep(2 * G_USEC_PER_SEC);
			siguran_exit(1);
		}
	} else{
		fprintf(stderr, _("Error: File does not exist: '%s'\n"), tmp_path);
		g_usleep(2 * G_USEC_PER_SEC);
		siguran_exit(1);
	}
	radio.trenutna = 0;
	radio_pustanje();
}
void prikazi_help(void){
	if (!setlocale(LC_ALL, "")){
		setlocale(LC_ALL, "C.UTF-8");
	}
	if (MB_CUR_MAX <= 1){
		setlocale(LC_ALL, "C.UTF-8");
	}
	utf8_ok = (MB_CUR_MAX > 1);
	bindtextdomain("tmuzika", "/usr/share/locale");
	textdomain("tmuzika");

	printf(_("tmuzika — terminal music player\n\n"));
	printf(_("Usage:\n"));
	printf(_("  tmuzika [options]\n\n"));
	printf(_("Options:\n"));
	printf(_("  -p, --play <file>         Play a song or load an m3u playlist\n"));
	printf(_("  -r, --radio <stream>      Play a radio stream or load an radio m3u\n"));
	printf(_("  -h, --help                Show this help message\n"));
	printf(_("\nAuthor: Ivan Janković — ivan.jankovic.unix@gmail.com\n"));
	printf(_("License: GPL v3 or later\n"));
	exit(0);
}
int main(int argc, char *argv[]){
	srand(time(NULL));
	// CLI POCETAK
	char *cli_radio_stream = NULL;
	static struct option long_opts[] ={
		{"play", required_argument, 0, 'p'},
		{"radio", required_argument, 0, 'r'},
		{"status",  no_argument, 0, 's' },
		{"title",  no_argument, 0, 't'},
		{"artist", no_argument, 0, 'a'},
		{"album",  no_argument, 0, 'l'},
		{"track",  no_argument, 0, 'n'},
		{"elapsed",no_argument, 0, 'e'},
		{"length", no_argument, 0, 'L'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};
	int opt;
	while ((opt = getopt_long(argc, argv, "p:r:staln:e:Lh", long_opts, NULL)) != -1){
		switch (opt){
			case 'p':{
				cli_mode = 1;
				strncpy(cli_start_path, optarg, PATH_MAX - 1);
				break;
			}
			case 'r':{
				cli_mode = 1;
				cli_radio_stream = g_strdup(optarg);
				break;
			}
			case 's':{
				if (tradio_status == TRADIO_PUSTEN){
					puts(_("tradio: playing"));
					return 0;
				} else if (tradio_status == TRADIO_PAUZIRAN){
					puts(_("tradio: paused"));
					return 0;
				}
				char buf[32] = "";
				FILE *f = fopen(TMUZIKA_TMP_DIR "/tmuzika_status", "r");
				if (f){
					if (fgets(buf, sizeof(buf), f)){
						buf[strcspn(buf, "\n")] = '\0';
					}
					fclose(f);
				}
				if (tmuzika_status == TMUZIKA_PUSTEN || strcmp(buf, "playing") == 0){
					puts(_("tmuzika: playing"));
				} else if (tmuzika_status == TMUZIKA_PAUZIRAN || strcmp(buf, "paused") == 0){
					puts(_("tmuzika: paused"));
				} else{
					puts(_("stopped"));
				}
				return 0;
			}
			case 't':{
				if ( tmuzika_status == TMUZIKA_PUSTEN){
					cli_print_file("tmuzika_title"); 
				} else{ 
					cli_print_file("tradio_title");
				} 
				return 0;
			}
			case 'a':{
				if ( tmuzika_status == TMUZIKA_PUSTEN){
					cli_print_file("tmuzika_artist"); 
				} else{ 
					cli_print_file("tradio_artist"); 
				}
				return 0;
			}
			case 'l':{
				if ( tmuzika_status == TMUZIKA_PUSTEN){
					cli_print_file("tmuzika_album"); 
				} else{ 
					cli_print_file("tradio_album"); 
				}
				return 0;
			}
			case 'n':{
				if ( tmuzika_status == TMUZIKA_PUSTEN){
					cli_print_file("tmuzika_track"); 
				} else{ 
					cli_print_file("tradio_track"); 
				}
				return 0;
			}
			case 'e':{
				if ( tmuzika_status == TMUZIKA_PUSTEN){
					cli_print_file("tmuzika_elapsed"); 
				} else{ 
					cli_print_file("tradio_elapsed");
				} 
				return 0;
			}
			case 'L':{
				if ( tmuzika_status == TMUZIKA_PUSTEN){
					cli_print_file("tmuzika_length"); 
				} else{ 
					cli_print_file("tradio_length");
				} 
				return 0;
			}
			case 'h':{
				prikazi_help();
				return 0;
			}
			default:{
				fprintf(stderr, "Nepoznata opcija\n");
				return 1;
			}
		}
	}
	// CLI KRAJ
	if (!setlocale(LC_ALL, "")){
		setlocale(LC_ALL, "C.UTF-8");
	}
	// Ako je locale C (ASCII), forsiraj UTF-8 fallback
	if (MB_CUR_MAX <= 1){
		setlocale(LC_ALL, "C.UTF-8");
	}
	utf8_ok = (MB_CUR_MAX > 1);
	bindtextdomain("tmuzika", "/usr/share/locale");
	textdomain("tmuzika");
	gst_init(&argc, &argv);
	pocetna_lista(&playlist);
	init_konfiguracioni_folder();
	// ako postoji ~/.tmuzika/glavna.m3u — učitaj je
	FILE *fp = fopen(putanja_osnovne_liste, "r");
	if (fp){
		fclose(fp);
		ucitavanje_m3u_liste(&playlist, putanja_osnovne_liste);
		ucitaj_poslednju_pesmu(); 
	}
	reprodukcioni_blok = gst_element_factory_make("playbin", "playbin");
	if (!reprodukcioni_blok){
		fprintf(stderr, _(" Playlist cannot be created\n "));
		return 1;
	}
	GstElement *audiosink = gst_element_factory_make("pulsesink", NULL);
	if (audiosink){
		g_object_set(reprodukcioni_blok, "audio-sink", audiosink, NULL);
	}
	g_object_set(reprodukcioni_blok, "volume", playlist.volume, NULL);
	poruke = gst_element_get_bus(reprodukcioni_blok);
	// ncurses konfiguracija
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	set_escdelay(1);//čekanje za ESC skoro nestaje (1ms)
	keypad(stdscr, TRUE);
	radio_init();
	if (has_colors()){
		start_color();
		use_default_colors();
		init_pair(CRVENA_BOJA, COLOR_RED, -1);
		init_pair(ZELENA_BOJA, COLOR_GREEN, -1);
		init_pair(ZUTA_BOJA, COLOR_YELLOW, -1);
		init_pair(PLAVA_BOJA, COLOR_BLUE, -1);
		init_pair(MAGENTA_BOJA, COLOR_MAGENTA, -1);
		init_pair(CYAN_BOJA, COLOR_CYAN, -1);
		init_pair(BELA_BOJA, COLOR_WHITE, -1);
		init_pair(SVETLO_ZELENA_BOJA, COLOR_GREEN, -1);
		init_pair(11, COLOR_WHITE, COLOR_RED);
		short SIVA_FG;
		if (COLORS >= 16){
			SIVA_FG = 8;
		} else{
			SIVA_FG = COLOR_WHITE;
		}
		init_pair(SIVA_BOJA, SIVA_FG, -1);
	}
	WINDOW *win = newwin(LINES, COLS, 0, 0);
	keypad(win, TRUE);
	wtimeout(win, 200);
	// CLI RADIO STREAM – automatski pusti pesmu ako postoji
	if (cli_radio_stream){
		cli_pusti_radio(cli_radio_stream);
		g_free(cli_radio_stream);
	}
	// CLI PATH – automatski pusti pesmu ako postoji
	if (cli_start_path[0]){
		char cli_path[MAX_DUZINA];
		strncpy(cli_path, cli_start_path, sizeof(cli_path) - 1);
		cli_path[sizeof(cli_path) - 1] = '\0';
		skracena_putanja_foldera(cli_path, sizeof(cli_path));
		struct stat st;
		if (stat(cli_path, &st) == 0){
			isprazni_playlistu(&playlist);
			zaustavi_pesmu();
			playlist.trenutna = 0;
			if (S_ISDIR(st.st_mode)){
				if (!dodavanje_celog_foldera(&playlist, cli_path)){
					fprintf(stderr, _(" Folder \"%s\" does not contain an audio file "), cli_start_path);
					g_usleep(2 * G_USEC_PER_SEC);
					siguran_exit(1);
            			}
			} else{
				const char *ext = strrchr(cli_path, '.');
				if (ext && g_ascii_strcasecmp(ext, ".m3u") == 0){
					ucitavanje_m3u_liste(&playlist, cli_path);
				} else if (provera_da_li_je_audio_fajl(cli_path)){
					dodavanje_fajla_u_listu(&playlist, cli_path);
				} else{
					fprintf(stderr, _(" %s selected file is not a .m3u or audio file "), cli_path);
					g_usleep(2 * G_USEC_PER_SEC);
					siguran_exit(1);
				}
			}
			if (playlist.stavke && playlist.stavke->len > 0){
				playlist.trenutna = 0;
				pustanje = 1;
				cli_pusti_izabranu_pesmu();
				cpu_ciscenje = 1;
			} else{
				fprintf(stderr, _(" No audio files to play in: %s\n "), cli_path);
				g_usleep(2 * G_USEC_PER_SEC);
				siguran_exit(1);
        		}
    		} else{
			fprintf(stderr, _(" Path not found: %s\n "), cli_path);
			g_usleep(2 * G_USEC_PER_SEC);
			siguran_exit(1);
    		}
	}
	// CLI KRAJ
	while (pokretanje){
		provera_zavrsetka_pesme();
		obrada_poruka();
		time_t vreme_pesme_sada = time(NULL);

		time_t sada_sat = time(NULL);
		if (sada_sat != poslednja_sekunda){
			poslednja_sekunda = sada_sat;
			if (radio_prozor){
				radio_cpu_ciscenje = 1;
			} else{
				cpu_ciscenje = 1;
			}
		}
		if (pustanje && vreme_pesme_sada != vreme_pesme_sekunda){
			vreme_pesme_sekunda = vreme_pesme_sada;
			gst_element_query_position(reprodukcioni_blok, GST_FORMAT_TIME, &cached_pos);
			gst_element_query_duration(reprodukcioni_blok, GST_FORMAT_TIME, &cached_dur);
			tmuzika_elapsed = (int)(cached_pos / GST_SECOND);
			tmuzika_length  = (int)(cached_dur / GST_SECOND);
			char elapsed_str[16], length_str[16];
			formatiraj_vreme(tmuzika_elapsed, elapsed_str, sizeof(elapsed_str));
			formatiraj_vreme(tmuzika_length, length_str, sizeof(length_str));
			FILE *f = fopen(TMUZIKA_TMP_DIR "/tmuzika_elapsed", "w");
			if (f){
				fprintf(f, "%s\n", elapsed_str); 
				fclose(f); 
			}
			f = fopen(TMUZIKA_TMP_DIR "/tmuzika_length", "w");
			if (f){ 
				fprintf(f, "%s\n", length_str); 
				fclose(f); 
			}
			sacuvaj_tmuzika_status();
			cpu_ciscenje = 1;
		}
		if (cpu_ciscenje){
			prikaz_programa(win);
			cpu_ciscenje = 0;
		}
		if (radio_prozor){
			if (radio_cpu_ciscenje){
				radio_prikaz(win);
				radio_cpu_ciscenje = 0;
			}
			int k = wgetch(win);
			if (k == ERR){
				usleep(10 * 1000);
				continue;
			} else if (k == 'k'){
				werase(win);
				wattron(win, COLOR_PAIR(SIVA_BOJA));
				box(win, 0, 0);
				wattroff(win, COLOR_PAIR(SIVA_BOJA));
				mvwprintw(win, 1, 1, _("Are you sure you want to exit? (y/N)"));
				wrefresh(win);
				wtimeout(win, -1);
				int c = wgetch(win);
				wtimeout(win, 200);
				if (c == 'y' || c == 'Y' || c == 'd' || c == 'D'){
					pokretanje = 0;
					break;
				} else{
					radio_cpu_ciscenje = 1;
					cpu_ciscenje = 1;
				}
			} else if (k == 27){
					radio_prozor = 0;
					radio_cpu_ciscenje = 1;
					cpu_ciscenje = 1; 
			} else if (k == KEY_UP){
				if (radio.trenutna > 0){
					radio.trenutna--;
				}
				if (radio.trenutna < radio.top_index){
					radio.top_index = radio.trenutna;
				}
				radio_cpu_ciscenje = 1;
			} else if (k == KEY_DOWN){
				if (radio.trenutna < (int)radio.stanice->len - 1){
					radio.trenutna++;
				}
				if (radio.trenutna >= radio.top_index + radio.visible_rows){
					radio.top_index = radio.trenutna - radio.visible_rows + 1;
				}
				if (radio.top_index < 0){
					radio.top_index = 0;
				}
				radio_cpu_ciscenje = 1;
			} else if (k == '+'){
				jacina_zvuka_radio(radio.volume + JACINA_KORAK);
			} else if (k == '-'){
				jacina_zvuka_radio(radio.volume - JACINA_KORAK);
        		} else if (k == '\n' || k == KEY_ENTER){
				char *playing_url = NULL;
				if (radio.trenutna >= 0 && radio.trenutna < (int)radio.stanice->len){
					RadioStanica *rs = g_ptr_array_index(radio.stanice, radio.trenutna);
					if (rs && rs->url){
						playing_url = g_strdup(rs->url);
					}
				}
				radio_pustanje();
				if (radio.backup_stanice){
					g_ptr_array_free(radio.stanice, TRUE);
					radio.stanice = radio.backup_stanice;
					radio.backup_stanice = NULL;
					radio.trenutna = 0;
					if (playing_url){
						for (guint i = 0; i < radio.stanice->len; i++){
							RadioStanica *rs = g_ptr_array_index(radio.stanice, i);
							if (rs && rs->url && strcmp(rs->url, playing_url) == 0){
								radio.trenutna = i;
								break;
							}
						}
					}
				}
				if (radio.trenutna >= 0 && radio.trenutna < (int)radio.stanice->len){
					radio.svira = radio.trenutna;
				}
				if (radio.svira < radio.visible_rows){
					radio.top_index = 0;
				} else{
					radio.top_index = radio.svira - radio.visible_rows + 1;
				}
				radio.trenutna = radio.svira;
				radio_cpu_ciscenje = 1;
				sacuvaj_poslednju_stanicu();
				if (playing_url){
					g_free(playing_url);
				}
			} else if (k == 'z'){
				radio_zaustavi();
				radio_cpu_ciscenje = 1;
			} else if (k == 'd'){
				radio_dodaj(win);
				radio_cpu_ciscenje = 1;
			} else if (k == 'x'){
				char potvrda[8] ={0};
				if (!radio.stanice || radio.stanice->len == 0){
					radio_cpu_ciscenje = 1;
					break;
				}
				RadioStanica *rs = g_ptr_array_index(radio.stanice, radio.trenutna);
				if (!rs){
					radio_cpu_ciscenje = 1;
					break;
				}
				char ime_kopija[512];
				const char *ime = rs->name ? rs->name : rs->url;
				snprintf(ime_kopija, sizeof(ime_kopija), "%s", ime);
				while (1){
					char prompt[512];
					snprintf(prompt, sizeof(prompt), _("Are you sure you want to delete radio station '%s'? (y/N): "), ime);
					int unos = zahtev_unosa_esc(win, prompt, potvrda, sizeof(potvrda));
					if (!unos){
						radio_cpu_ciscenje = 1;
						break;
					}
					if (potvrda[0] == '\0' || potvrda[0] == 'n' || potvrda[0] == 'N'){
						radio_cpu_ciscenje = 1;
						break;
					}
					if (potvrda[0] == 'y' || potvrda[0] == 'Y' || potvrda[0] == 'd' || potvrda[0] == 'D'){
						if (radio.trenutna == radio.svira && radio.online){
							radio_zaustavi();
						}
						radio_obrisi_izabran();
						char msg[512];
						snprintf(msg, sizeof(msg), _(" The radio station '%s' has been deleted "), ime_kopija);
						footer_prikaz_poruka(win, msg);
						radio_cpu_ciscenje = 1;
						footer_privremeni = FALSE;
						footer_lista(win);
						break;
					}
				}
			} else if (k == KEY_DC){
				char potvrda[8] ={0};
				if (!radio.stanice || radio.stanice->len == 0){
					radio_cpu_ciscenje = 1;
					break;
				}
				while (1){
					const char *prompt = _("Are you sure you want to delete all radio stations? (y/N): ");
					int unos = zahtev_unosa_esc(win, prompt, potvrda, sizeof(potvrda));
					radio_zaustavi();
					if (!unos){
						radio_cpu_ciscenje = 1;
						break;
					}
					if (potvrda[0] == '\0' || potvrda[0] == 'n' || potvrda[0] == 'N'){
						radio_cpu_ciscenje = 1;
						break;
					}
					if (potvrda[0] == 'y' || potvrda[0] == 'Y' || potvrda[0] == 'd' || potvrda[0] == 'D'){
						radio_obrisi_listu();
						footer_prikaz_poruka(win, _(" All radio stations have been deleted "));
						gint64 start = g_get_monotonic_time();
						while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){
							provera_zavrsetka_pesme();
							g_usleep(50 * 1000);
						}
						radio_cpu_ciscenje = 1;
						footer_privremeni = FALSE;
						footer_lista(win);
						break;
					}
				}
			} else if (k == '?'){
				pomoc_komande(win);
				radio_cpu_ciscenje = 1;
			} else if (k == 'm'){
				trenutni_rezim = REZIM_FILE_MANAGER;
				FileManager fm ={0};
				getcwd(fm.trenutni_put, sizeof(fm.trenutni_put));
				fm_cpu_ciscenje = TRUE;
				fm_ucitaj(&fm);
				touchwin(stdscr);
				wrefresh(stdscr);
				while (trenutni_rezim == REZIM_FILE_MANAGER){
					fm_crtaj(win, &fm);
					int k = wgetch(win);
					if (fm_key(&fm, k) < 0){
						trenutni_rezim = REZIM_PLAYER;
					}
				}
				if (fm.backup_stavke){
					g_ptr_array_free(fm.stavke, TRUE);
					fm.stavke = fm.backup_stavke;
				}
				g_ptr_array_free(fm.stavke, TRUE);
				radio_cpu_ciscenje = 1;
    			} else if (k == ' '){
				pauziraj_radio();
				radio_cpu_ciscenje = 1;
			} else if (k == 's'){
				char query[256];
				while (1){
					int unos = zahtev_unosa_esc(win, _("Search radio station: "), query, sizeof(query));
					if (!unos || query[0] == '\0'){
						if (radio.backup_stanice){
							g_ptr_array_free(radio.stanice, TRUE);
							radio.stanice = radio.backup_stanice;
							radio.backup_stanice = NULL;
						}
						if (radio.svira >= 0 && radio.svira < (int)radio.stanice->len){
							radio.trenutna = radio.svira;
							if (radio.trenutna < radio.visible_rows){
								radio.top_index = 0;
							} else{
								radio.top_index = radio.trenutna - radio.visible_rows + 1;
							}
						} else{
							radio.trenutna = 0;
							radio.top_index = 0;
						}
						radio_cpu_ciscenje = 1;
						break;
					}
					if (!radio.backup_stanice){
						radio.backup_stanice = g_ptr_array_new_with_free_func(radio_stanica_free);
						for (guint i = 0; i < radio.stanice->len; i++){
							RadioStanica *rs = g_ptr_array_index(radio.stanice, i);
							RadioStanica *kopija = g_malloc(sizeof(RadioStanica));
							kopija->name = g_strdup(rs->name);
							kopija->url  = g_strdup(rs->url);
							g_ptr_array_add(radio.backup_stanice, kopija);
						}
					}
					g_ptr_array_set_size(radio.stanice, 0);
					gchar *q = g_ascii_strdown(query, -1);
					for (guint i = 0; i < radio.backup_stanice->len; i++){
						RadioStanica *rs = g_ptr_array_index(radio.backup_stanice, i);
						gchar *low = g_ascii_strdown(rs->name, -1);
						if (strstr(low, q)){
							RadioStanica *kopija = g_malloc(sizeof(RadioStanica));
							kopija->name = g_strdup(rs->name);
							kopija->url  = g_strdup(rs->url);
							g_ptr_array_add(radio.stanice, kopija);
						}
						g_free(low);
					}
					g_free(q);
					if (radio.stanice->len == 0){
						footer_prikaz_poruka(win, _(" Station not found: %s "), query);
						usleep(1500 * 1000);
						footer_privremeni = 0;
						radio.trenutna = 0;
						radio.top_index = 0;
						radio_cpu_ciscenje = 1;
						continue; // ide na sledeći unos
					} else{
						radio.trenutna = 0;
						if (radio.trenutna < radio.visible_rows){
							radio.top_index = 0;
						} else{
							radio.top_index = radio.trenutna - radio.visible_rows + 1;
						}
						radio_cpu_ciscenje = 1;
						break;
					}
				}
			} else if (k == 'v' || k == 'V'){
				if (radio.svira >= 0 && radio.svira < (int)radio.stanice->len){
					int max_rows = radio.visible_rows > 0 ? radio.visible_rows : 5;
					if (radio.svira < radio.top_index){
						radio.top_index = radio.svira;
					} else if (radio.svira >= radio.top_index + max_rows){
						radio.top_index = radio.svira - max_rows + 1;
					}
					radio.trenutna = radio.svira;
					radio_cpu_ciscenje = 1;
				}
			} else if (k == 'u'){			
				char path[MAX_DUZINA];
				while (1){
					int unos_radio = zahtev_unosa_esc(win, _("Load (.m3u) playlist: "), path, sizeof(path));	
					if (!unos_radio){
						radio_cpu_ciscenje = 1;
						break;
					}
					if (unos_radio && path[0]){
						skracena_putanja_foldera(path, sizeof(path));
						struct stat st;
						if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)){
							footer_prikaz_poruka(win, _(" m3u playlist not found: %s "), path);	
							gint64 start = g_get_monotonic_time();
							while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){
								g_usleep(50 * 1000); // 50ms
							}
							radio_cpu_ciscenje = 1;
							footer_privremeni = FALSE;
							footer_lista(win);
							continue;
						} else{
							guint ucitan_m3u = ucitavanje_m3u_liste_radio(path);
							if(ucitan_m3u > 0){
								radio.trenutna = 0;
								sacuvaj_m3u_liste_radio();
								footer_prikaz_poruka(win, _(" %s m3u playlist loaded "), path);	
								gint64 start = g_get_monotonic_time();
								while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){	
									g_usleep(50 * 1000);
								}
								radio_cpu_ciscenje = 1;
								footer_privremeni = FALSE;
								footer_lista(win);
								break;
							} else{
								footer_prikaz_poruka(win, _(" %s Playlist not loaded, no radio streams found "), path);	
								gint64 start = g_get_monotonic_time();
								while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){	
									g_usleep(50 * 1000);
								}
								footer_privremeni = FALSE;
                						footer_lista(win);
							}
						}
					}
				}
			} else if (k == '>'){
				pusti_sledecu_stanicu();
			} else if (k == '<'){
				pusti_prethodnu_stanicu();
			} else if (k == KEY_HOME){
				pusti_prvu_stanicu();
			} else if (k == KEY_END){
				pusti_poslednju_stanicu();
			}
			continue;
		}
		int ch = wgetch(win);
		int visible = COLS - 4;
		int remaining;
		if (ch == ERR){
			usleep(10 * 1000);
			continue;
		} else if (ch == 'r' || ch == 'R'){
			pustanje = 0;
			radio_prozor = 1;
			radio_cpu_ciscenje = 1;
		} else if (ch == 'k'){
			werase(win);
			wattron(win, COLOR_PAIR(SIVA_BOJA));
			box(win, 0, 0);
			wattroff(win, COLOR_PAIR(SIVA_BOJA));
			mvwprintw(win, 1, 1, _("Are you sure you want to exit? (y/N)"));
			wrefresh(win);
			wtimeout(win, -1);
			int c = wgetch(win);
			wtimeout(win, 200);
			if (c == 'y' || c == 'Y' || c == 'd' || c == 'D'){
				pokretanje = 0;
				break;
			} else{
				cpu_ciscenje = 1;
			}
		} else if (ch == '\n' || ch == KEY_ENTER){
			const char *uri = NULL;
			if (playlist.stavke && playlist.trenutna >= 0 && playlist.trenutna < (int)playlist.stavke->len){
				uri = g_ptr_array_index(playlist.stavke, playlist.trenutna);
			}
			pusti_izabranu_pesmu();
			horizontal_scroll = 0;
			if (playlist.backup_stavke && uri){
				vrati_playlistu_i_pozicioniraj(&playlist, uri);
			}
			sacuvaj_poslednju_pesmu();
			cpu_ciscenje = 1;
		} else if (ch == 'z'){
			zaustavi_pesmu();
			cpu_ciscenje = 1;
		} else if (ch == ' '){
			pauziraj_pesmu();
			cpu_ciscenje = 1;
		} else if (ch == '>'){
			sledeca_pesma();
			horizontal_scroll = 0;
			cpu_ciscenje = 1;
		} else if (ch == '<'){
			prethodna_pesma();
			horizontal_scroll = 0;
			cpu_ciscenje = 1;
		} else if (ch == '+'){
			jacina_zvuka(playlist.volume + JACINA_KORAK);
			cpu_ciscenje = 1;
		} else if (ch == '-'){
			jacina_zvuka(playlist.volume - JACINA_KORAK);
			cpu_ciscenje = 1;
		} else if (ch == KEY_RIGHT){
			ubrzaj_pesmu(SKOK_SEKUNDE);
			horizontal_scroll = 0;
			cpu_ciscenje = 1;
		} else if (ch == KEY_LEFT){
			ubrzaj_pesmu(-SKOK_SEKUNDE);
			horizontal_scroll = 0;
			cpu_ciscenje = 1;
		} else if (ch == 'm'){
			trenutni_rezim = REZIM_FILE_MANAGER;
			FileManager fm ={0};
			getcwd(fm.trenutni_put, sizeof(fm.trenutni_put));
			fm_ucitaj(&fm);
			touchwin(stdscr);
    			wrefresh(stdscr);
			while (trenutni_rezim == REZIM_FILE_MANAGER){
				fm_crtaj(win, &fm);
				int k = wgetch(win);
				if (fm_key(&fm, k) < 0){
					trenutni_rezim = REZIM_PLAYER;
				}
			}
			if (fm.backup_stavke){
				g_ptr_array_free(fm.stavke, TRUE);
				fm.stavke = fm.backup_stavke;
			}
			g_ptr_array_free(fm.stavke, TRUE);
			cpu_ciscenje = 1;
		} else if (ch == 'q'){			
			char path[MAX_DUZINA];
			while (1){
				int unos = zahtev_unosa_esc(win, _("Save (.m3u) playlist: "), path, sizeof(path));
				if (!unos){
					cpu_ciscenje = 1;
					break;
					}
				if (unos && path[0]){
					cuvanje_m3u_liste(&playlist, path);
					footer_prikaz_poruka(win, " %s%s", path, _(" .m3u playlist saved "));
					gint64 start = g_get_monotonic_time();
					while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){
						provera_zavrsetka_pesme();
						g_usleep(50 * 1000);
					}
					cpu_ciscenje = 1;
					footer_privremeni = FALSE;
					footer_lista(win);
					break;
				}
			}
		} else if (ch == 'u'){			
			char path[MAX_DUZINA];
			while (1){
				int unos = zahtev_unosa_esc(win, _("Load (.m3u) playlist: "), path, sizeof(path));
				provera_zavrsetka_pesme();
				if (!unos){
					cpu_ciscenje = 1;
					break;
				}
				if (unos && path[0]){
					skracena_putanja_foldera(path, sizeof(path));
					struct stat st;
					if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)){
						footer_prikaz_poruka(win, _(" m3u playlist not found: %s "), path);
						gint64 start = g_get_monotonic_time();
						while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){
							provera_zavrsetka_pesme();
							g_usleep(50 * 1000);
						}
						cpu_ciscenje = 1;
						footer_privremeni = FALSE;
						footer_lista(win);
						continue;
					} else{
						guint ucitan_m3u = ucitavanje_m3u_liste(&playlist, path);
						if(ucitan_m3u > 0){
							playlist.trenutna = 0;
							cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
							footer_prikaz_poruka(win, _(" %s m3u playlist loaded "), path);
							gint64 start = g_get_monotonic_time();
							while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){
								provera_zavrsetka_pesme();
								g_usleep(50 * 1000);
							}
							cpu_ciscenje = 1;
							footer_privremeni = FALSE;
							footer_lista(win);
							break;
						} else{
							footer_prikaz_poruka(win, _(" %s Playlist not loaded, contents stream URL's "), path);	
							gint64 start = g_get_monotonic_time();
							while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){	
								provera_zavrsetka_pesme();
								g_usleep(50 * 1000);
							}
							footer_privremeni = FALSE;
                					footer_lista(win);
						}
					}
				}
			}
		} else if (ch == KEY_UP){
			if (playlist.stavke->len > 0 && playlist.trenutna > 0){
				playlist.trenutna--;
				horizontal_scroll = 0;
				cpu_ciscenje = 1;
			}
		} else if (ch == KEY_DOWN){
			if (playlist.stavke->len > 0 && playlist.trenutna < (int)playlist.stavke->len - 1){
				playlist.trenutna++;
				horizontal_scroll = 0;
				cpu_ciscenje = 1;
			}
		} else if (ch == '?'){
			pomoc_komande(win);
			cpu_ciscenje = 1;
		} else if (ch == 's'){
			char query[256];
			while (1){
				provera_zavrsetka_pesme();
				int unos = zahtev_unosa_esc(win, _("Search for a song, album or artist: "), query, sizeof(query));
				provera_zavrsetka_pesme();
				if (!unos || query[0] == '\0'){
					if (playlist.backup_stavke){
						int trenutno_svira = playlist.svira;
						g_ptr_array_free(playlist.stavke, TRUE);
						playlist.stavke = playlist.backup_stavke;
						playlist.backup_stavke = NULL;
						if (trenutno_svira < (int)playlist.stavke->len){
							playlist.trenutna = trenutno_svira;
						} else{
							playlist.trenutna = 0;
						}
					}
					cpu_ciscenje = 1;
					break;
				}
				if (!playlist.backup_stavke){
					playlist.backup_stavke = g_ptr_array_new_with_free_func(g_free);
					for (guint i = 0; i < playlist.stavke->len; i++){
						g_ptr_array_add(playlist.backup_stavke,
						g_strdup(g_ptr_array_index(playlist.stavke, i)));
					}
				}
				g_ptr_array_set_size(playlist.stavke, 0);
				gchar *q = g_ascii_strdown(query, -1);
				for (guint i = 0; i < playlist.backup_stavke->len; i++){
					const char *uri = g_ptr_array_index(playlist.backup_stavke, i);
					gchar *f_ime = gst_uri_get_location(uri);
					const char *hay = f_ime ? f_ime : uri;
					gchar *low = g_ascii_strdown(hay, -1);
					if (strstr(low, q)){
						g_ptr_array_add(playlist.stavke, g_strdup(uri));
					}
					g_free(low);
					if (f_ime){
						g_free(f_ime);
					}
				}
				g_free(q);
				if (playlist.stavke->len == 0){
					footer_prikaz_poruka(win, _(" Song not found: %s "), query);
					usleep(1500 * 1000);
					footer_privremeni = 0;
					playlist.trenutna = 0;
					cpu_ciscenje = 1;
					continue;
				} else{
					playlist.trenutna = 0;
					cpu_ciscenje = 1;
					break;
				}
			}				
		} else if (ch == KEY_DC){
			char potvrda[8] ={0};
			while (1){
				const char *prompt = _("Are you sure you want to delete m3u? (y/N): ");
				int unos = zahtev_unosa_esc(win, prompt, potvrda, sizeof(potvrda));
				provera_zavrsetka_pesme();
				if (!unos){
					cpu_ciscenje = 1;
					break;
				}
				if (potvrda[0] == '\0' || potvrda[0] == 'n' || potvrda[0] == 'N'){
					cpu_ciscenje = 1;
					break;
				}
				if (potvrda[0] == 'y' || potvrda[0] == 'Y' || potvrda[0] == 'd' || potvrda[0] == 'D'){
					isprazni_playlistu(&playlist);
					zaustavi_pesmu();
					cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
					footer_prikaz_poruka(win, _(" m3u playlist is empty "));
					gint64 start = g_get_monotonic_time();
					while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){
						provera_zavrsetka_pesme();
						g_usleep(50 * 1000);
					}
					cpu_ciscenje = 1;
					footer_privremeni = FALSE;
					footer_lista(win);
					break;
				}
			}
		} else if (ch == 'x'){
			char potvrda[8] ={0};
			const char *trenutna_pesma = g_ptr_array_index(playlist.stavke, playlist.trenutna);
			gchar *f_ime = gst_uri_get_location(trenutna_pesma);
			const char *ime = f_ime ? g_path_get_basename(f_ime) : trenutna_pesma;
			while (1){

				char prompt[512];
				snprintf(prompt, sizeof(prompt), _("Are you sure you want to delete song '%s'? (y/N): "), ime);
				int unos = zahtev_unosa_esc(win, prompt, potvrda, sizeof(potvrda));
				provera_zavrsetka_pesme();
				if (!unos){
					cpu_ciscenje = 1;
					break;
				}
				if (potvrda[0] == '\0' || potvrda[0] == 'n' || potvrda[0] == 'N'){
					cpu_ciscenje = 1;
					break;
				}
				if (potvrda[0] == 'y' || potvrda[0] == 'Y' || potvrda[0] == 'd' || potvrda[0] == 'D'){
					ukloni_trenutnu_pesmu(&playlist);
					zaustavi_pesmu();
					cuvanje_m3u_liste(&playlist, putanja_osnovne_liste);
					char msg[512];
					snprintf(msg, sizeof(msg), _(" The song '%s' has been deleted "), ime);
					footer_prikaz_poruka(win, msg);
					gint64 start = g_get_monotonic_time();
					while (g_get_monotonic_time() - start < 2 * G_USEC_PER_SEC){
						provera_zavrsetka_pesme();
						g_usleep(50 * 1000);
					}
					cpu_ciscenje = 1;
					footer_privremeni = FALSE;
					footer_lista(win);
					break;
				}
			}
			if (f_ime) g_free(f_ime);
		} else if (ch == 'v' || ch == 'V'){
			if (playlist.svira >= 0 && playlist.svira < (int)playlist.stavke->len){
				playlist.trenutna = playlist.svira;
				cpu_ciscenje = 1;
			}
		} else if (ch == KEY_HOME){
			playlist.trenutna = 0;
			horizontal_scroll = 0;
			cpu_ciscenje = 1;
		} else if (ch == KEY_END){
			if (playlist.stavke->len > 0){
				playlist.trenutna = playlist.stavke->len - 1;
				horizontal_scroll = 0;
				cpu_ciscenje = 1;
			}
		} else if (ch == 'p' || ch == 'P'){
			ponovi_pesmu = !ponovi_pesmu;
			if (ponovi_pesmu){
				ponovi_listu = 0;
				nasumicno_pustanje = 0;
			}
			cpu_ciscenje = 1;
		} else if (ch == 'l' || ch == 'L'){
			ponovi_listu = !ponovi_listu;
			if (ponovi_listu){
				ponovi_pesmu = 0;
				nasumicno_pustanje = 0;
			}
			cpu_ciscenje = 1;
		} else if (ch == 'e' || ch == 'E'){
			nasumicno_pustanje = !nasumicno_pustanje;
			if (nasumicno_pustanje){
				ponovi_pesmu = 0;
				ponovi_listu = 0;
				napravi_nasumican_red();
			}
			cpu_ciscenje = 1;
		} else if (ch == 'n'){
			remaining = horizontal_scroll;
			int step = visible / 2;
			if (step < 1){ 
				step = 1;
			}
			if (remaining < step){ 
				step = remaining;
			}
			horizontal_scroll -= step;
			if (horizontal_scroll < 0){ 
				horizontal_scroll = 0;
			}
			cpu_ciscenje = 1;
		} else if (ch == 'b'){
			remaining = max_line_length - horizontal_scroll - visible;
			int step = visible / 2;
			if (step < 1){ 
				step = 1;
			}
			if (remaining < step){
				step = remaining;
			}
			horizontal_scroll += step;
			if (horizontal_scroll > max_line_length - visible){
				horizontal_scroll = max_line_length - visible;
			}
			cpu_ciscenje = 1;
		}
	}
	// Ciscenje
	zaustavi_pesmu();
	gst_element_set_state(reprodukcioni_blok, GST_STATE_NULL);
	if (poruke){
		gst_object_unref(poruke);
		poruke = NULL;
	}
	if (reprodukcioni_blok){
		gst_element_set_state(reprodukcioni_blok, GST_STATE_NULL);
		gst_object_unref(reprodukcioni_blok);
		reprodukcioni_blok = NULL;
	}
	oslobadjanje_liste(&playlist);
	delwin(win);
	endwin();
	return 0;
}