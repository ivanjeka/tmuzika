#ifndef DEFINICIJE_H
#define DEFINICIJE_H

#include <glib.h>
#include <glib/gi18n.h>   // za gettext
#include <gst/gst.h>      // za GST_SECOND
#include <limits.h>       // za PATH_MAX, MAX dužine putanja i buffer-a

// --- Lokalizacija --- 
//#define _(STRING) gettext(STRING)

//--- Opšte konstante ---
#define MAX_DUZINA 4096             // maksimalna veličina stringa / buffer-a
#define JACINA_KORAK 0.05           // korak za jačinu zvuka
#define SKOK_SEKUNDE (10 * GST_SECOND) // koliko sekundi preskačeš unapred / unazad

// --- Boje --- 
#define CRVENA_BOJA 1
#define ZELENA_BOJA 2
#define ZUTA_BOJA 3
#define PLAVA_BOJA 4
#define MAGENTA_BOJA 5
#define CYAN_BOJA 6
#define BELA_BOJA 7
#define SIVA_BOJA 10
#define SVETLO_ZELENA_BOJA 21

// --- Bookmark / Cache --- 
#define MAX_BOOKMARKS 9
#define TMUZIKA_CACHE_MAGIC 0x544D5A4B
#define TMUZIKA_CACHE_VERSION 1
#define TMUZIKA_TMP_DIR "/tmp"

// --- Dodatna zaštita --- 
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#endif
