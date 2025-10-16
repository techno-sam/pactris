/* spel.h
 *
 * Deze module bevat de `toestand` enum.
 * Hiermee geven pacman en tetris aan of het spel door moet gaan.
 */

#ifndef _SPEL_H
#define _SPEL_H

typedef enum {
    AAN_HET_SPELEN,
    GEWONNEN,
    VERLOREN,
    NEUTRAAL_KLAAR
} toestand;

#endif
