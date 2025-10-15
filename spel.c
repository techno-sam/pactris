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
#include "pacman.h"
#include "config.h"

typedef struct {
    pacman *pm_data;
    WINDOW *pm_win;
    int pm_hoogte;
    int pm_breedte;
} spel;

WINDOW *centered_window(int hoogte, int breedte) {
    int y0 = (LINES - hoogte) / 2;
    int x0 = (COLS - breedte) / 2;
    return newwin(hoogte, breedte, y0, x0);
}

spel *maak_spel(rooster *pacman_veld) {
    spel *sp = malloc(sizeof(spel));
    if (sp == NULL) {
        perror("maak_spel");
        return NULL;
    }

    int hoogte, breedte;
    sp->pm_data = pm_maak(pacman_veld, &hoogte, &breedte);

    if (sp->pm_data == NULL) {
        free(sp);
        return NULL;
    }

    sp->pm_win = centered_window(hoogte, breedte);
    sp->pm_hoogte = hoogte;
    sp->pm_breedte = breedte;

    return sp;
}

void teken_spel(spel *sp) {
    pm_teken(sp->pm_win, sp->pm_data);

    wnoutrefresh(sp->pm_win);
    doupdate();
}

void speel(spel *sp) {
    teken_spel(sp);

    clock_t pm_laatste_stap = clock();

    int speel = 1;
    while (speel) {
        // 1. toetsen
        int toets = getch();
        if (toets != ERR) {
            int gebruikt = pm_toets(toets, sp->pm_data);

            if (!gebruikt) {
                switch (toets) {
                    case 27: // ESC
                        speel = 0;
                        break;
                    case KEY_RESIZE:
                        delwin(sp->pm_win);
                        sp->pm_win = centered_window(sp->pm_hoogte, sp->pm_breedte);

                        for (int y = 0; y < LINES; y++) {
                            move(y, 0);
                            for (int x = 0; x < COLS; x++) {
                                addch(' ');
                            }
                        }

                        break;
                }
            }
        }

        // 2. stap, 25 Hz
        clock_t nu = clock();
        clock_t delta = nu - pm_laatste_stap;
        if (delta > CLOCKS_PER_SEC / PM_HERTZ) {
            pm_laatste_stap = nu;
            speel &= pm_stap(sp->pm_data);
        }

        // 3. teken
        teken_spel(sp);
    }
}

void spel_klaar(spel *sp) {
    pm_klaar(sp->pm_data);
    delwin(sp->pm_win);
    free(sp);
}

int main(int argc, char *argv[]) {
    // 1. Controleer dat er een pacmanbestand is opgegeven op de command line.
    if (argc != 2) {
        fprintf(stderr, "gebruik: ./spel assets/pacman.txt\n");
        return 1;
    }

    // 2. Open het pacmanbestand en lees het rooster.
    FILE *fh = fopen(argv[1], "r");
    if (fh == NULL) {
        perror("main");
        return 1;
    }
    rooster *pacman_veld = rooster_lees(fh);
    fclose(fh);

    // 3. Bepaal of het lezen van het rooster is gelukt.
    if (pacman_veld == NULL) {
        fprintf(stderr, "Kan rooster niet maken.\n");
        return 1;
    }

    // 4. Initialiseer ncurses
    initscr();
    cbreak();              // zodat je kunt onderbreken met Ctrl+C
    keypad(stdscr, TRUE);  // luister ook naar extra toetsen zoals pijltjes
    noecho();              // druk niet de letters af die je intypt
    nodelay(stdscr, TRUE); // getch wacht niet
    curs_set(0);           // verberg de cursor
    init_kleuren();

    // 5. Maak het spel
    spel *spel = maak_spel(pacman_veld);
    if (spel == NULL) {
        fprintf(stderr, "Kan spel niet maken.\n");
        return 1;
    }

    // 6. Speel het spel.
    speel(spel);

    // 7. Sluit af.
    getch(); // handig voor debugging
    spel_klaar(spel);
    endwin();
    return 0;
}
