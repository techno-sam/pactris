/* config.h
 *
 * Bevat configuratiewaardes om het spel makelijk aan te passen
 */

#ifndef _CONFIG_H
#define _CONFIG_H

// Pacman config

// stappen/seconde
#define PM_HERTZ 25 
// maximum aantal levens
#define PM_MAX_LEVENS 5
// levens aan het begin van het spel
#define PM_BEGIN_LEVENS 3
// hoe lang knipperen de spoken aan het einde van de bangperiode
#define PM_BANG_WAARSCHUWING (2*PM_HERTZ)
// hoe lang worden spoken bang
#define PM_BANG_STAPPEN (10*PM_HERTZ)

#endif
