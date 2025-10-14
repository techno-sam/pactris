/*
 * Naam: S.F.L. Wagenaar
 * UvAnetID: 16409752
 * Studie: BSc Informatica
 *
 * spel.c
 * - Lorem Ipsum
 */

// om `nanosleep` beschikbaar te maken
#define _POSIX_C_SOURCE 199309L

#include <assert.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <ncurses.h>

#include "kleuren.h"
#include "subspel.h"
#include "pacman.h"

typedef struct {
    subspel spel;
    WINDOW *win;
} spel_data;

typedef struct {
    int n_spellen;
    spel_data *spellen;
} spel;

WINDOW *centered_window(int hoogte, int breedte) {
    int y0 = (LINES - hoogte) / 2;
    int x0 = (COLS - breedte) / 2;
    return newwin(hoogte, breedte, y0, x0);
}

spel maak_spel(void) {
    int n = 1;
    spel_data *spellen = malloc(n * sizeof(spel_data));

    int win_x, win_y;
    subspel pacman = pacman_maak_subspel(&win_x, &win_y);
    spel_data pacman_data = {
        .spel = pacman,
        .win = centered_window(win_y, win_x),
    };
    spellen[0] = pacman_data;

    spel spel = {
        .n_spellen = n,
        .spellen = spellen,
    };

    return spel;
}

void speel(void) {
    spel spel = maak_spel();

    for (int i = 0; i < spel.n_spellen; i++) {
        spel_data *sub = &spel.spellen[i];
        sub->spel.init(sub->win, sub->spel.data);
    }

    while (1) {
        int toets = getch();
        if (toets != ERR) {
            for (int i = 0; i < spel.n_spellen; i++) {
                spel_data *sub = &spel.spellen[i];
                if (sub->spel.toets(sub->win, toets, sub->spel.data)) {
                    break;
                }
            }
        }

        for (int i = 0; i < spel.n_spellen; i++) {
            spel_data *sub = &spel.spellen[i];
            sub->spel.teken(sub->win, TEKEN_VERS, sub->spel.data);
            wrefresh(sub->win);
        }
    refresh();
    }

    for (int i = 0; i < spel.n_spellen; i++) {
        spel_data *sub = &spel.spellen[i];
        sub->spel.klaar(sub->spel.data);
        wborder(sub->win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        wrefresh(sub->win);
        delwin(sub->win);
    }
}

int main(void) {
    // Initialiseer ncurses
    initscr();
    cbreak();              // zodat je kunt onderbreken met Ctrl+C
    keypad(stdscr, TRUE);  // luister ook naar extra toetsen zoals pijltjes
    noecho();              // druk niet de letters af die je intypt
    nodelay(stdscr, TRUE); // getch wacht niet
    curs_set(0);           // verberg de cursor

    init_kleuren();

    speel();

    getch(); // handig voor debugging
    endwin();
    return 0;
}
