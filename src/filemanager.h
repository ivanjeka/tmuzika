#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <glib.h>     // GPtrArray
#include <limits.h>   // PATH_MAX

/* --- Režimi FileManager-a --- */
enum Rezim {
	REZIM_PLAYER,
	REZIM_FILE_MANAGER
};

/* --- Tipovi akcija --- */
typedef enum {
    ACTION_NONE,
    ACTION_COPY,
    ACTION_CUT,
    ACTION_DELETE
} ActionType;

/* --- Clipboard režimi --- */
typedef enum {
    CLIPBOARD_NONE = 0,
    CLIPBOARD_COPY,
    CLIPBOARD_CUT
} ClipboardMode;

/* --- Glavna struktura FileManager --- */
typedef struct {
    char trenutni_put[PATH_MAX];       // trenutni folder
    GPtrArray *stavke;                 // lista fajlova/foldera
    int selektovan;                    // selektovani indeks
    int offset;                         // za prikaz u ncurses prozoru
    GPtrArray *backup_stavke;          // backup lista za undo ili restore
    GPtrArray *clipboard;               // privremeni clipboard
    ClipboardMode clipboard_mode;       // COPY ili CUT
    char *clipboard_path;               // putanja fajla/foldera u clipboard
    ActionType clipboard_action;        // zadnja akcija na clipboardu
    char *undo_path_src;                // izvor putanja za undo
    char *undo_path_dest;               // destinacija za undo
    ActionType last_action;             // zadnja akcija u FileManager-u
    GPtrArray *selektovani;            // niz selektovanih stavki (za multiple selection)
} FileManager;

#endif /* FILEMANAGER_H */
