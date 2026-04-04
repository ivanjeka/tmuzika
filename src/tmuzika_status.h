#ifndef TMUZIKA_STATUS_H
#define TMUZIKA_STATUS_H

#include <stdint.h>

typedef enum {
	TMUZIKA_ZAUSTAVLJEN = 0,
	TMUZIKA_PUSTEN,
	TMUZIKA_PAUZIRAN
} TmuzikaStatus;

extern TmuzikaStatus tmuzika_status;

extern char tmuzika_title[256];
extern char tmuzika_artist[128];
extern char tmuzika_album[128];

extern int tmuzika_track;
extern int tmuzika_elapsed;
extern int tmuzika_length; 

#endif
