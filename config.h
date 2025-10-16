/* config.h
 *
 * Bevat configuratiewaardes om het spel makelijk aan te passen
 */

#ifndef _CONFIG_H
#define _CONFIG_H

// Pacman config

// voor debuggen, of pacman gespeeld wordt
#define PACMAN 1
// stappen/seconde
#define PM_HERTZ 25 
// maximum aantal levens
#define PM_MAX_LEVENS 5
// levens aan het begin van het spel
#define PM_BEGIN_LEVENS 3
// hoe lang knipperen de spoken aan het einde van de bangperiode
#define PM_BANG_WAARSCHUWING (2 * PM_HERTZ)
// hoe lang worden spoken bang per gegeten supervoedsel
#define PM_BANG_STAPPEN (8 * PM_HERTZ)
// maximum hoeveelheid tijd dat spoken bang kunnen zijn
#define PM_MAX_BANG_STAPPEN (16 * PM_HERTZ)

#endif
