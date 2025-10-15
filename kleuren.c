#include "kleuren.h"
#include <ncurses.h>

#define COLOR_BRIGHT 8

void init_kleuren(void) {
    if (start_color() == ERR) {
        return;
    }

    if (can_change_color()) {
        init_color(COLOR_YELLOW, 1000, 1000, 0);
        init_color(COLOR_WHITE, 1000, 1000, 1000);
        init_color(17, 1000, 725, 875);
        init_color(18, 1000, 725, 271);

        init_pair(K_PINKY, 17, COLOR_BLACK);
        init_pair(K_CLYDE, 18, COLOR_BLACK);
    } else {
        init_pair(K_PINKY, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(K_CLYDE, COLOR_GREEN, COLOR_BLACK);
    }

    init_pair(K_PACMAN, COLOR_YELLOW, COLOR_BLACK);
    init_pair(K_MUUR, COLOR_BLUE, COLOR_BLUE);
    init_pair(K_HUIS_MUUR, COLOR_BLUE, COLOR_BLACK);
    init_pair(K_HUIS_DEUR, COLOR_WHITE, COLOR_BLACK);
    init_pair(K_HARTJE, COLOR_RED, COLOR_BLACK);
    init_pair(K_HARTJE_DOOD, COLOR_BLACK | COLOR_BRIGHT, COLOR_BLACK);

    init_pair(K_BLINKY, COLOR_RED | COLOR_BRIGHT, COLOR_BLACK);
    init_pair(K_INKY, COLOR_CYAN, COLOR_BLACK);
}

void wkleur_aan(WINDOW *win, kleur kleur) {
    wattron(win, COLOR_PAIR(kleur));
}

void wkleur_uit(WINDOW *win, kleur kleur) {
    wattroff(win, COLOR_PAIR(kleur));
}
