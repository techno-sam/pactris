#include <ncurses.h>
#include <stdlib.h>

#include "pacman.h"
#include "subspel.h"
#include "rooster.h"

typedef struct {
    rooster *veld;
} pacman_spel;

void pm_init(WINDOW *win, pacman_spel *data) {
    box(win, 0, 0);
}

int pm_toets(WINDOW *win, int toets, pacman_spel *data) {

    move(2, 3);
    wprintw(win, "Toets: %d", toets);

    return 0;
}

void pm_stap(WINDOW *win, pacman_spel *data) {}

void pm_teken(WINDOW *win, teken_mode mode, pacman_spel *data) {}

void pm_klaar(pacman_spel *data) {
    if (data->veld) {
        rooster_klaar(data->veld);
    }
    free(data);
}

subspel pacman_maak_subspel(int *win_x, int *win_y) {
    pacman_spel *data = malloc(sizeof(pacman_spel));
    data->veld = NULL; // TODO: load

    subspel spel = {
        .init = (subs_init) &pm_init,
        .toets = (subs_toets) &pm_toets,
        .stap = (subs_stap) &pm_stap,
        .teken = (subs_teken) &pm_teken,
        .klaar = (subs_klaar) &pm_klaar,
        .data = data
    };

    return spel;
}
