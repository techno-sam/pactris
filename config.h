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
#define PM_BANG_STAPPEN (10 * PM_HERTZ)

// maximum hoeveelheid tijd dat spoken bang kunnen zijn
#define PM_MAX_BANG_STAPPEN (20 * PM_HERTZ)

///////////////////
// Tetris config //
///////////////////

// stappen/seconde
#define TR_HERTZ 4

// breedte van de kuil
#define TR_BREEDTE 10

// hoogte van de kuil
#define TR_HOOGTE 20

// na hoeveel stukken mag een tetromino herhalen
#define TR_UNIEK_PERIODE 1

#if TR_UNIEK_PERIODE < 0 || TR_UNIEK_PERIODE > 2
#error "TR_UNIEK_PERIODE moet 0, 1, of 2 zijn"
#endif

#endif
