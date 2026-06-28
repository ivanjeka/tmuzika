#include "pomoc_komande.h"

#include <stdio.h>
#include <libintl.h>

#include "boje.h"
#include "komande.h"

#define K(x) tmuzika_key_to_string(x)

void pomoc_komande(WINDOW *win){
	int ch;
	wtimeout(win, 200);
	// TMUZIKA
	char linije_tmuzika[32][128];
	int t = 0;
	snprintf(linije_tmuzika[t++], 128, "%s", _("TMUZIKA"));
	snprintf(linije_tmuzika[t++], 128, " ");
	snprintf(linije_tmuzika[t++], 128, "ENTER = %s", _("play song"));
	snprintf(linije_tmuzika[t++], 128, "SPACE = %s", _("pause / continue"));
	snprintf(linije_tmuzika[t++], 128, "[ / ] = %s", _("left / right"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_stop),_("stop"));
	snprintf(linije_tmuzika[t++], 128, "<- / -> = %s", _("next / previous"));
	snprintf(linije_tmuzika[t++], 128, "+ / - = %s", _("volume"));
	snprintf(linije_tmuzika[t++], 128, "< / > = %s", _("seek +/- 10s"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_repete_song),_("repeat song"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_repete_all),_("repeat playlist"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_shutlle),_("shuffle"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_search),_("search"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_radio),_("radio"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_file_manager),_("file manager"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_save_m3u),_("save m3u"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_load_m3u),_("load m3u"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_current_song),_("current song"));
	snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_delete_song),_("delete song"));
	snprintf(linije_tmuzika[t++], 128, "DELETE = %s", _("delete all"));
	snprintf(linije_tmuzika[t++], 128, "HOME = %s", _("go to the first song"));
	snprintf(linije_tmuzika[t++], 128, "END = %s", _("go to the last song"));
	//snprintf(linije_tmuzika[t++],128,"%s = %s",K(komande.tmuzika_exit),_("exit"));

	// FILE MANAGER
	int levi_count = t;
	const char *desni_panel[] ={
		_("FILE MANAGER"),
		_(" "),
		_("- ENTER = open / add"),
		_("- ESC = parent"),
		_("- [ / ] = left / right"),
		_("- ctrl+h = toggle hidden files"),
		_("- f/F = add folder / add current folder"),
		_("- m = mark files / folders"),
		_("- ctrl+a = select all"),
		_("- d = add multiple filess"),
		_("- s = search"),
		_("- ctrl+n = new folder"),
		_("- ctrl+t = new file"),
		_("- F2 = rename file / folder"),
		_("- c = copy"),
		_("- x = cut"),
		_("- v = paste"),
		_("- u = undo"),
		_("- DELETE = remove file / folder"),
		_("- ctrl+b = add bookmarks"),
		_("- ctrl+d = remove bookmark"),
		_("- NUMBER 1 - 9 = go to bookmarks"),
		_("- BACKSPACE = back")
	};
	int desni_count = sizeof(desni_panel)/sizeof(desni_panel[0]);
	char linije_radio[32][128];
	int r = 0;
	// TRADIO
	snprintf(linije_radio[r++],128,"%s",_("RADIO"));
	snprintf(linije_radio[r++],128," ");
	snprintf(linije_radio[r++], 128, "ENTER = %s", _("play radio station"));
	snprintf(linije_radio[r++], 128, "SPACE = %s", _("pause / continue"));
	snprintf(linije_radio[r++],128,"%s = %s",K(komande.tradio_stop),_("stop"));
	snprintf(linije_radio[r++], 128, "<- / -> = %s", _("next / previous"));
	snprintf(linije_radio[r++], 128, "+ / - = %s", _("volume"));
	snprintf(linije_radio[r++],128,"%s = %s",K(komande.tradio_add),_("add radio station"));
	snprintf(linije_radio[r++],128,"%s = %s",K(komande.tradio_search),_("search"));
	snprintf(linije_radio[r++],128,"%s = %s",K(komande.tradio_save_m3u),_("save m3u"));
	snprintf(linije_radio[r++],128,"%s = %s",K(komande.tradio_load_m3u),_("load m3u"));
	snprintf(linije_radio[r++],128,"%s = %s",K(komande.tradio_current_song),_("current radio station"));
	snprintf(linije_radio[r++],128,"%s = %s",K(komande.tradio_delete_song),_("delete radio station"));
	snprintf(linije_radio[r++], 128, "DELETE = %s", _("delete all"));
	snprintf(linije_radio[r++], 128, "HOME = %s", _("go to the first radio station"));
	snprintf(linije_radio[r++], 128, "END = %s", _("go to the last radio station"));
	snprintf(linije_radio[r++], 128, "BACKSPACE = %s", _("back"));
	int sredina_count = r;
	int offset_panel = 0;
	int offset_radio = 0;

	// GLOBAL
	char linije_global[16][128];
	int g = 0;
	snprintf(linije_global[g++],128,"%s",_("GLOBAL"));
	snprintf(linije_global[g++],128," ");
	snprintf(linije_global[g++],128,"o = %s",_("options"));
	//snprintf(linije_global[g++],128,"h = %s",_("reset keys"));
	snprintf(linije_global[g++],128,"b = %s",_("reset colors"));
	snprintf(linije_global[g++],128,"? = %s",_("help"));
	snprintf(linije_global[g++],128,"k = %s",_("exit app"));
	int global_count = g;
	while (1){
		provera_zavrsetka_pesme();
		werase(win);
		int height, width;
		getmaxyx(win, height, width);
		int panel_height = (height - 3) / 2;
		int max_rows_panel = panel_height;
		int max_rows_radio = height - 2 - panel_height - 1;
		// BORDER
		wattron(win, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
		box(win, 0, 0);
		wattroff(win, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
		int inner_w = width - 2;
		int mid = inner_w / 2;
		// vertikalna linija
		wattron(win, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
		for (int y = 1; y <= panel_height; y++){
			mvwaddch(win, y, mid + 1, ACS_VLINE);
		}
		wattroff(win, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
		// TMUZIKA
		for (int i = 0; i < max_rows_panel && i + offset_panel < levi_count; i++){
			mvwprintw(win, i + 1, 2, "%s", linije_tmuzika[i + offset_panel]);
		}
		// FILE MANAGER
		for (int i = 0; i < max_rows_panel && i + offset_panel < desni_count; i++){
			mvwprintw(win, i + 1, mid + 3, "%s", desni_panel[i + offset_panel]);
		}
		// strelice panel
		int max_off_panel = (levi_count > desni_count ? levi_count : desni_count) - max_rows_panel;
		if (max_off_panel < 0){
			max_off_panel = 0;
		}
		if (offset_panel > 0){
			mvwaddch(win, 1, width - 2, ACS_UARROW);
		}
		if (offset_panel < max_off_panel){
			mvwaddch(win, panel_height, width - 2, ACS_DARROW);
		}
		// horizontalna linija
		int sep = panel_height + 1;
		wattron(win, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
		for (int x = 1; x < width - 1; x++){
			mvwaddch(win, sep, x, ACS_HLINE);
		}
		wattroff(win, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
		// RADIO
		int radio_mid = inner_w / 2;
		// vertikalna linija za RADIO/GLOBAL
		wattron(win, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
		for (int y = sep + 1; y < height - 1; y++){
			mvwaddch(win, y, radio_mid + 1, ACS_VLINE);
		}
		wattroff(win, COLOR_PAIR(TMUZIKA_BORDER_PAIR));
		// RADIO
		for (int i = 0; i < max_rows_radio && i + offset_radio < sredina_count; i++){
			mvwprintw(win, sep + 1 + i, 2, "%s", linije_radio[i + offset_radio]);
		}
		// GLOBAL
		for (int i = 0; i < max_rows_radio && i + offset_radio < global_count; i++){
			mvwprintw(win, sep + 1 + i, radio_mid + 3, "%s", linije_global[i + offset_radio]);
		}
		// strelice radio
		int max_off_radio = sredina_count - max_rows_radio;
		if (max_off_radio < 0){
			max_off_radio = 0;
		}
		if (offset_radio > 0){
			mvwaddch(win, sep + 1, width - 2, ACS_UARROW);
		}
		if (offset_radio < max_off_radio){
			mvwaddch(win, height - 2, width - 2, ACS_DARROW);
		}
		// FOOTER
		footer_pomoc(win);
		wrefresh(win);
		ch = wgetch(win);
		if (ch == 127 || ch == KEY_BACKSPACE){
			return;
		}
		// scroll panel
		if (ch == KEY_PPAGE && offset_panel > 0){
			offset_panel--;
		}
		if (ch == KEY_NPAGE && offset_panel < max_off_panel){
			offset_panel++;
		}
		// scroll radio
		if (ch == KEY_UP && offset_radio > 0){
			offset_radio--;
		}
		if (ch == KEY_DOWN && offset_radio < max_off_radio){
			offset_radio++;
		}
	}
}