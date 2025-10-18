/* tetris.h
 *
 * Deze module bevat alle spellogica van Tetris.
 * Een tetrisspel bestaat uit een kuil, en een vallend tetromino.
 */

#ifndef _TETRIS_H
#define _TETRIS_H

#include <ncurses.h>
#include <stdio.h>

#include "spel.h"

struct tetris_data;
typedef struct tetris_data tetris;


/* Maak een tetrisspel
 *
 * na_verwijder: een (NULLable) callback om aan te roepen nadat regels verwijderd zijn
 *               (arg is aantal regels)
 * hoogte: een pointer om de gevraagde vensterhoogte in te schrijven
 * breedte: een pointer om de gevraagde vensterhoogte in te schrijven
 *
 * Uitvoer: een pointer naar een tetrisspel, of NULL als er iets fout gaat
 *
 * Side effects:
 * - hoogte en breedte worden gezet
 */
tetris *tr_maak(int_callback na_verwijder, int *hoogte, int *breedte);


/* Geef alle resources vrij die zijn gealloceerd voor een tetrisspel.
 * De spelpointer is na aanroep van deze functie niet meer bruikbaar.
 *
 * data: een pointer naar de speldata
 */
void tr_klaar(tetris *data);


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
int tr_toets(int toets, tetris *data);


/* Simuleer een stap van het spel
 *
 * data: de speldata
 *
 * Uitvoer: de toestand waarin het spel moet verkeren
 *
 * Side effects:
 * - de speldata wordt veranderd
 */
toestand tr_stap(tetris *data);


/* Teken het spel
 *
 * win: het venster om op te tekenen
 * data: de speldata
 *
 * Side effects:
 * - tekst wordt naar het venster geschreven
 */
void tr_teken(WINDOW *win, const tetris *data);

/* Kijk wat de score is
 *
 * data: de speldata
 *
 * Uitvoer: de score
 */
int tr_score(const tetris *data);

#endif
