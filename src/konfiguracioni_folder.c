#include "konfiguracioni_folder.h"
#include "definicije.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

extern char poslednja_pustena_stanica[MAX_DUZINA];
extern char radio_fajl[PATH_MAX];
extern char poslednja_pustena_stanica[MAX_DUZINA];
extern char poslednja_pustena_pesma[MAX_DUZINA];
extern char putanja_cache_liste[PATH_MAX];
extern char putanja_osnovne_liste[MAX_DUZINA];
extern char konfiguracioni_folder[MAX_DUZINA];

static int tmuzika_postoji(const char *path){
	struct stat st;
	return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}
static int kopiraj_stare_fajlove_tmuzika(const char *src, const char *dst){
	FILE *in = fopen(src, "rb");
	if (!in){
		return -1;
	}
	FILE *out = fopen(dst, "wb");
	if (!out){
		fclose(in);
		return -1;
	}
	char buf[8192];
	size_t n;
	while ((n = fread(buf, 1, sizeof(buf), in)) > 0){
		fwrite(buf, 1, n, out);
	}
	fclose(in);
	fclose(out);
	return 0;
}
static void kopiraj_direktorijum_sa_poddirektorijumima_tmuzika(const char *src, const char *dst){
	mkdir(dst, 0700);
	DIR *d = opendir(src);
	if (!d){ 
		return;
	}
	struct dirent *ent;
	while ((ent = readdir(d)) != NULL){
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0){
			continue;
		}
		char src_path[1024];
		char dst_path[1024];
		snprintf(src_path, sizeof(src_path), "%s/%s", src, ent->d_name);
		snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, ent->d_name);
		struct stat st;
		if (stat(src_path, &st) == 0){
			if (S_ISDIR(st.st_mode)){
				kopiraj_direktorijum_sa_poddirektorijumima_tmuzika(src_path, dst_path);
			} else if (S_ISREG(st.st_mode)){
				kopiraj_stare_fajlove_tmuzika(src_path, dst_path);
			}
		}
	}
	closedir(d);
}
static void migracija_staru_konfiguraciju_tmuzika(const char *old_path, const char *new_path){
	if (!tmuzika_postoji(old_path)){
		return;
	}
	if (tmuzika_postoji(new_path)){
		return;
	}
	mkdir(new_path, 0700);
	kopiraj_direktorijum_sa_poddirektorijumima_tmuzika(old_path, new_path);
	char marker[512];
	snprintf(marker, sizeof(marker), "%s/.migrated", new_path);
	FILE *f = fopen(marker, "w");
	if (f){
		fprintf(f, "migrated from %s\n", old_path);
		fclose(f);
	}
}
void init_konfiguracioni_folder(){
	const char *home = getenv("HOME");
	if (!home){
		home = ".";
	}
	char old_path[256];
	char new_config[256];
	char new_data[256];
	char new_cache[256];
	char old_config[512];
	char backup_path[PATH_MAX];
	snprintf(old_path, sizeof(old_path), "%s/.tmuzika", home);
	snprintf(new_config, sizeof(new_config), "%s/.config/tmuzika", home);
	snprintf(new_data, sizeof(new_data), "%s/.local/share/tmuzika", home);
	snprintf(new_cache, sizeof(new_cache), "%s/.cache/tmuzika", home);
	migracija_staru_konfiguraciju_tmuzika(old_path, new_data);
	snprintf(old_config, sizeof(old_config), "%s/config", old_path);
	migracija_staru_konfiguraciju_tmuzika(old_config, new_config);
	mkdir(new_config, 0700);
	mkdir(new_data, 0700);
	mkdir(new_cache, 0700);
	snprintf(putanja_osnovne_liste, sizeof(putanja_osnovne_liste), "%s/glavna.m3u", new_data);
	snprintf(radio_fajl, sizeof(radio_fajl), "%s/radio.tradio", new_data);
	snprintf(poslednja_pustena_stanica, sizeof(poslednja_pustena_stanica), "%s/poslednje_slusana_stanica", new_data);
	snprintf(poslednja_pustena_pesma, sizeof(poslednja_pustena_pesma), "%s/poslednje_slusano", new_data);
	snprintf(putanja_cache_liste, sizeof(putanja_cache_liste), "%s/glavna.cache", new_cache);
	snprintf(konfiguracioni_folder, sizeof(konfiguracioni_folder), "%s", new_config);
	snprintf(backup_path, sizeof(backup_path), "%s/.tmuzika.bak", home);
	if (access(old_path, F_OK) == 0 && access(backup_path, F_OK) != 0){
		rename(old_path, backup_path);
	}
}