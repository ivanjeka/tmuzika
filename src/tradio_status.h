#ifndef TRADIO_STATUS_H
#define TRADIO_STATUS_H

#include <stdint.h>

typedef enum {
    TRADIO_ZAUSTAVLJEN,
    TRADIO_PUSTEN,
    TRADIO_PAUZIRAN
} TradioStatus;

extern TradioStatus tradio_status;

extern char tradio_title[256];
extern char tradio_artist[128];
extern char tradio_album[128];

extern int tradio_track;
extern int tradio_elapsed;
extern int tradio_length;

#endif