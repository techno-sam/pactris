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
        init_color(CK_PINKY, 1000, 725, 875);
        init_color(CK_CLYDE, 1000, 725, 271);
        init_color(CK_PM_VENSTER, 1000, 1000, 1000);

        init_pair(K_PINKY, CK_PINKY, COLOR_BLACK);
        init_pair(K_CLYDE, CK_CLYDE, COLOR_BLACK);
        init_pair(K_PM_VENSTER, CK_PM_VENSTER, COLOR_BLACK);

        init_color(COLOR_BLACK | COLOR_BRIGHT, 100, 100, 100);

        int tetromino_kleuren[7][3] = {
            {  789, 1000,  160},
            {  846,    0,  122},
            {  920,  491,  184},
            {  302,  224,  948},
            {  296,  915,  748},
            {  915,  296,  548},
            {  296,  625,  915},
        };

        for (int i = 0; i < 7; i++) {
            int *k = tetromino_kleuren[i];
            init_color(CK_TR_TETROMINO_0 + i, k[0], k[1], k[2]);
            init_pair(
                K_TETROMINO_0 + i,
                CK_TR_TETROMINO_0 + i,
                CK_TR_TETROMINO_0 + i
            );
        }
    } else {
        init_pair(K_PINKY, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(K_CLYDE, COLOR_GREEN, COLOR_BLACK);
        init_pair(K_PM_VENSTER, COLOR_WHITE, COLOR_BLACK);

        init_pair(K_TETROMINO_0, COLOR_YELLOW, COLOR_YELLOW);
        init_pair(K_TETROMINO_1, COLOR_RED, COLOR_RED);
        init_pair(K_TETROMINO_2, COLOR_GREEN, COLOR_GREEN);
        init_pair(K_TETROMINO_3, COLOR_CYAN, COLOR_CYAN);
        init_pair(K_TETROMINO_4, COLOR_BLUE, COLOR_BLUE);
        init_pair(K_TETROMINO_5, COLOR_MAGENTA, COLOR_MAGENTA);
        init_pair(K_TETROMINO_6, COLOR_WHITE, COLOR_WHITE);
    }

    init_pair(K_PACMAN, COLOR_YELLOW, COLOR_BLACK);
    init_pair(K_MUUR, COLOR_BLUE, COLOR_BLUE);
    init_pair(K_HUIS_MUUR, COLOR_BLUE, COLOR_BLACK);
    init_pair(K_HUIS_DEUR, COLOR_WHITE, COLOR_BLACK);
    init_pair(K_HARTJE, COLOR_RED, COLOR_BLACK);
    init_pair(K_HARTJE_DOOD, COLOR_BLACK | COLOR_BRIGHT, COLOR_BLACK);

    init_pair(K_BLINKY, COLOR_RED | COLOR_BRIGHT, COLOR_BLACK);
    init_pair(K_INKY, COLOR_CYAN, COLOR_BLACK);

    init_pair(K_BANG, COLOR_GREEN, COLOR_BLACK);
    init_pair(K_BANG_ALT, COLOR_WHITE, COLOR_GREEN);

    init_pair(K_TR_VENSTER, COLOR_WHITE, COLOR_BLACK);
    init_pair(K_TR_MUUR, COLOR_BLUE, COLOR_BLACK);
    init_pair(K_TR_ACHTERGROND, COLOR_BLACK, COLOR_BLACK);
    init_pair(K_TR_ACHTERGROND_ALT, COLOR_BLACK | COLOR_BRIGHT, COLOR_BLACK | COLOR_BRIGHT);
}

void wkleur_aan(WINDOW *win, kleur kleur) {
    wattron(win, COLOR_PAIR(kleur));
}

void wkleur_uit(WINDOW *win, kleur kleur) {
    wattroff(win, COLOR_PAIR(kleur));
}
