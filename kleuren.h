/** TODO: HEADER COMMENT */

#ifndef _KLEUREN_H
#define _KLEUREN_H

#include <ncurses.h>
typedef enum {
    K_DEFAULT = 0,
    K_PACMAN,
    K_MUUR,
    K_HARTJE,
    K_HARTJE_DOOD,
} kleur;

void init_kleuren(void);

void wkleur_aan(WINDOW *win, kleur kleur);

void wkleur_uit(WINDOW *win, kleur kleur);

#endif
