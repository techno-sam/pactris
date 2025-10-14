#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

#include "pacman.h"
#include "kleuren.h"
#include "rooster.h"

#define MAX_LEVENS 3
#define SCORE_HOOGTE 3

const int DX[] =           {   1,   0,  -1,   0 };
const int DY[] =           {   0,   1,   0,  -1 };
const char SPELER_CHAR[] = { '<', '^', '>', 'v' };
const char SPELER_ALT[] =  { '-', '|', '-', '|' };

typedef enum {
    GEEN = -1,
    RECHTS = 0,
    OMLAAG = 1,
    LINKS  = 2,
    OMHOOG = 3,
} richting;

typedef struct {
    int x;
    int y;
    richting rot;
} entity;

struct pacman_data {
    rooster *veld;
    int breedte;
    int hoogte;
    int stappen;

    int levens;
    int begin_voedsel;
    int gegeten;

    richting doel_rot;
    entity speler;
};

pacman *pm_maak(rooster *veld, int *hoogte, int *breedte) {
    pacman *data = malloc(sizeof(pacman));
    if (data == NULL) {
        perror("pm_maak");
        return NULL;
    }

    data->veld = veld;
    data->breedte = rooster_breedte(data->veld);
    data->hoogte = rooster_hoogte(data->veld);
    data->stappen = 0;

    *breedte = data->breedte + 2;
    *hoogte = data->hoogte + 2 + SCORE_HOOGTE;

    data->levens = MAX_LEVENS;

    // vul het veld met voedsel
    data->begin_voedsel = 1;
    data->gegeten = 1; // het vak waar de speler op begint telt als virtuele voedsel
    for (int y = 0; y < data->hoogte; y++) {
        for (int x = 0; x < data->breedte; x++) {
            char c = rooster_kijk(data->veld, x, y);

            switch (c) {
                case '~': // speciale voedsel-vrije plek
                    rooster_plaats(data->veld, x, y, ' ');
                    break;
                case ' ':
                    rooster_plaats(data->veld, x, y, '.');
                    data->begin_voedsel++;
                    break;
            }
        }
    }

    rooster_zoek(data->veld, '*', &data->speler.x, &data->speler.y);
    if (data->speler.x == -1) {
        fprintf(stderr, "geen beginplaats in pacmanveld\n");
        pm_klaar(data);
        return NULL;
    }
    rooster_plaats(data->veld, data->speler.x, data->speler.y, ' ');
    data->speler.rot = RECHTS;
    data->doel_rot = GEEN;

    return data;
}

int pm_toets(int toets, pacman *data) {
    switch (toets) {
        case 'w': data->doel_rot = OMHOOG; return 1;
        case 'a': data->doel_rot = LINKS;  return 1;
        case 's': data->doel_rot = OMLAAG; return 1;
        case 'd': data->doel_rot = RECHTS; return 1;
    }

    return 0;
}

int bewandelbaar(char c) {
    return c == ' ' || c == '.';
}

/* Probeer een stap te zetten
 *
 * veld: het veld om door te lopen
 * ent: de entity (speler of spook) dat moet lopen
 *
 * Uitvoer: de karakter waar `ent` nu op staat, of '\0' als geen stap mogelijk was
 * Side effects: verplaatst `ent`
 */
char entity_stap(const rooster *veld, entity *ent) {
    int x0 = ent->x;
    int y0 = ent->y;
    richting rot = ent->rot;

    int x1 = x0 + DX[rot];
    int y1 = y0 + DY[rot];

    char c = rooster_kijk(veld, x1, y1);

    if (bewandelbaar(c)) {
        ent->x = x1;
        ent->y = y1;
        return c;
    } else {
        return '\0';
    }
}

void verander_richting(pacman *data) {
    // probeer van richting te veranderen
    int x0 = data->speler.x;
    int y0 = data->speler.y;
    richting rot = data->speler.rot;

    if (data->doel_rot != GEEN) {
        if (data->doel_rot == rot) {
            data->doel_rot = GEEN;
        } else {
            int x1 = x0 + DX[data->doel_rot];
            int y1 = y0 + DY[data->doel_rot];
            char c = rooster_kijk(data->veld, x1, y1);

            if (bewandelbaar(c)) {
                rot = data->speler.rot = data->doel_rot;
                data->doel_rot = GEEN;
            }
        }
    }
}

int pm_stap(pacman *data) {
    verander_richting(data);

    char c = entity_stap(data->veld, &data->speler);
    if (c == '.') {
        rooster_plaats(data->veld, data->speler.x, data->speler.y, ' ');
        data->gegeten++;
    }

    data->stappen++;

    return data->levens > 0;
}

void teken_veld(WINDOW *win, const rooster *rp) {
    int breedte = rooster_breedte(rp);
    int hoogte = rooster_hoogte(rp);

    for (int y = 0; y < hoogte; y++) {
        wmove(win, y + 1 + SCORE_HOOGTE, 1);

        for (int x = 0; x < breedte; x++) {
            char c0 = rooster_kijk(rp, x, y);
            chtype c = c0;

            switch (c0) {
                case '#': c |= A_BOLD | A_DIM | COLOR_PAIR(K_MUUR); break;
                case '.': c = ACS_BULLET; break;
            }

            waddch(win, c);
        }
    }
}

void pm_teken(WINDOW *win, pacman *data) {
    box(win, 0, 0);

    mvwprintw(win, 1, 1, "Score: %d / %d", data->gegeten, data->begin_voedsel);

    wkleur_aan(win, K_HARTJE);
    for (int i = 0; i < data->levens; i++) {
        mvwprintw(win, 1, data->breedte - 2 - 3*i, "<3");
    }
    wkleur_uit(win, K_HARTJE);
    wkleur_aan(win, K_HARTJE_DOOD);
    for (int i = data->levens; i < MAX_LEVENS; i++) {
        mvwprintw(win, 1, data->breedte - 2 - 3*i, "<3");
    }
    wkleur_uit(win, K_HARTJE_DOOD);

    wmove(win, 3, 1);
    for (int x = 0; x < data->breedte; x++) {
        waddch(win, ACS_HLINE);
    }

    teken_veld(win, data->veld);

    //wkleur_aan(win, K_PACMAN);
    const char *speler_chars = (data->stappen % 2) ? SPELER_ALT : SPELER_CHAR;
    mvwaddch(
        win,
        data->speler.y + 1 + SCORE_HOOGTE,
        data->speler.x + 1,
        speler_chars[data->speler.rot] | COLOR_PAIR(K_PACMAN) | A_BOLD
    );
    //wkleur_uit(win, K_PACMAN);
}

void pm_klaar(pacman *data) {
    if (data->veld) {
        rooster_klaar(data->veld);
    }
    free(data);
}
