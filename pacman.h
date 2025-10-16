/* pacman.h
 *
 * Deze module bevat alle spellogica van Pacman.
 * Een pacmanspel bestaat uit een rooster (zie rooster.h),
 * een speler, en vier spoken.
 *
 * De spoken hebben allemaal verschillend doelgedrag:
 * Blinky: volgt de speler
 * Pinky: probeert vier vakjes voor de speler te komen
 * Inky: loopt van de speler weg als Blinky ver van de speler is,
 *       anders volgt ie de speler
 * Clyde: loopt naar de linkeronderhoek als de speler binnen 8 stappen,
 *        anders volgt ie de speler
 */

#ifndef _PACMAN_H
#define _PACMAN_H

#include <ncurses.h>
#include <stdio.h>

#include "rooster.h"
#include "spel.h"

struct pacman_data;
typedef struct pacman_data pacman;


/* Maak een pacmanspel
 *
 * veld: de speelveld. Deze functie neemt ownership.
 * hoogte: een pointer om de gevraagde vensterhoogte in te schrijven
 * breedte: een pointer om de gevraagde vensterhoogte in te schrijven
 *
 * Uitvoer: een pointer naar een pacmanspel, of NULL als er iets fout gaat
 *
 * Side effects:
 * - hoogte en breedte worden gezet
 */
pacman *pm_maak(rooster *veld, int *hoogte, int *breedte);


/* Geef alle resources vrij die zijn gealloceerd voor een pacmanspel.
 * De spelpointer is na aanroep van deze functie niet meer bruikbaar.
 *
 * data: een pointer naar de speldata
 */
void pm_klaar(pacman *data);


/* Reageer op een gedrukte toets
 *
 * toets: de toets die gedrukt is
 * data: de speldata
 *
 * Uitvoer: 1 als de toets gebruikt is, anders 0
 *
 * Side effects:
 * - de speldata kan aangepast worden
 */
int pm_toets(int toets, pacman *data);


/* Simuleer een stap van het spel
 *
 * data: de speldata
 *
 * Uitvoer: de toestand waarin het spel moet verkeren
 *
 * Side effects:
 * - de speldata wordt veranderd
 */
toestand pm_stap(pacman *data);


/* Teken het spel
 *
 * win: het venster om op te tekenen
 * data: de speldata
 *
 * Side effects:
 * - tekst wordt naar het venster geschreven
 */
void pm_teken(WINDOW *win, const pacman *data);

#endif
