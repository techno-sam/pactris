/* config.h
 *
 * Bevat configuratiewaardes om het spel makelijk aan te passen
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/////////////////////
// Algemene config //
/////////////////////

// De ruimte tussen de twee spellen
#define SPEL_MARGE 4

// voor debuggen, of pacman gespeeld wordt
#define PACMAN 1

// voor debuggen, of tetris gespeeld wordt
#define TETRIS 1

///////////////////
// Pacman config //
///////////////////

// stappen/seconde
#define PM_HERTZ 25 

// maximum aantal levens
#define PM_MAX_LEVENS 5

// levens aan het begin van het spel
#define PM_BEGIN_LEVENS 3

// hoe lang knipperen de spoken aan het einde van de bangperiode
#define PM_BANG_WAARSCHUWING (2 * PM_HERTZ)

// hoe lang worden spoken bang per gegeten supervoedsel
#if TETRIS
#define PM_BANG_STAPPEN (12 * PM_HERTZ)
#else
#define PM_BANG_STAPPEN (8 * PM_HERTZ)
#endif

// maximum hoeveelheid tijd dat spoken bang kunnen zijn
#if TETRIS
#define PM_MAX_BANG_STAPPEN (30 * PM_HERTZ)
#else
#define PM_MAX_BANG_STAPPEN (16 * PM_HERTZ)
#endif

///////////////////
// Tetris config //
///////////////////

// stappen/seconde
#define TR_HERTZ 4

// breedte van de kuil
#define TR_BREEDTE 10

// hoogte van de kuil
#define TR_HOOGTE 20

#endif
