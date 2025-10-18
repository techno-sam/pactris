/* kleuren.h
 *
 * Deze module verzorgt de registratie van alle gebruikte kleuren.
 */

#ifndef _KLEUREN_H
#define _KLEUREN_H

#include <ncurses.h>

typedef enum {
    K_DEFAULT = 0,
    K_PM_VENSTER,
    K_PACMAN,
    K_MUUR,
    K_HUIS_MUUR,
    K_HUIS_DEUR,
    K_HARTJE,
    K_HARTJE_DOOD,
    K_BLINKY,
    K_PINKY,
    K_INKY,
    K_CLYDE,
    K_BANG,
    K_BANG_ALT,

    K_TR_VENSTER,
    K_TR_MUUR,
    K_TR_ACHTERGROND,
    K_TR_ACHTERGROND_ALT,
    K_TETROMINO_0,
    K_TETROMINO_1,
    K_TETROMINO_2,
    K_TETROMINO_3,
    K_TETROMINO_4,
    K_TETROMINO_5,
    K_TETROMINO_6,
} kleur;

typedef enum {
    CK_PINKY = 17,
    CK_CLYDE = 18,
    CK_PM_VENSTER = 19,

    CK_TR_TETROMINO_0 = 20,
    CK_TR_TETROMINO_1 = 21,
    CK_TR_TETROMINO_2 = 22,
    CK_TR_TETROMINO_3 = 23,
    CK_TR_TETROMINO_4 = 24,
    CK_TR_TETROMINO_5 = 25,
    CK_TR_TETROMINO_6 = 26,
    CK_TR_ACHTERGROND_ALT = 27,
} custom_kleur;

/* Registreer alle nodige kleuren
 *
 * Side effects:
 * - een aantal rgb waardes worden aangepast
 * - COLOR_PAIRs worden gemaakt voor alle kleuren
 */
void init_kleuren(void);

/* Zet een tekstkleur in een venster aan
 *
 * win: het venster om aam te passen
 * kleur: de kleur
 *
 * Side effects:
 * - de tekstkleur van het venster wordt aangezet
 */
void wkleur_aan(WINDOW *win, kleur kleur);

/* Zet een tekstkleur in een venster uit
 *
 * win: het venster om aam te passen
 * kleur: de kleur
 *
 * Side effects:
 * - de tekstkleur van het venster wordt uitgezet
 */
void wkleur_uit(WINDOW *win, kleur kleur);

#endif
