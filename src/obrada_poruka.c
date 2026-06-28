#include "obrada_poruka.h"

#include <stdio.h>
#include <gst/gst.h>
#include <glib.h>
#include <libintl.h>

#include "tmuzika.h"

#define _(String) gettext(String)

extern int radio_prozor;
extern int ponovi_pesmu;
extern int ponovi_listu;
extern int nasumicno_pustanje;
extern int cpu_ciscenje;

extern GstBus *poruke;


extern struct {
	GPtrArray *stavke;
	int trenutna;
} playlist;

extern void pusti_izabranu_pesmu(void);
extern void sledeca_pesma(void);
extern void sledeca_nasumicna_pesma(void);
extern void zaustavi_pesmu(void);

void obrada_poruka(){
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