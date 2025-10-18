/*
 * Naam: S.F.L. Wagenaar
 * UvAnetID: 16409752
 * Studie: BSc Informatica
 *
 * spel.c
 * - het spel heeft 3 modi: 'allebei', 'pacman', en 'tetris'
 *   - `./spel allebei assets/pacman.txt`: speel Pacman & Tetris tegelijkertijd
 *   - `./spel pacman assets/pacman.txt`: speel alleen Pacman
 *   - `./spel tetris`: speel alleen Tetris
 * - als het spel te moeilijk of makkelijk is zijn veel aspecten configureerbaar in `config.h`
 * - een rij verwijderen in Tetris heeft hetzelfde effect als een supervoedsel ('*') te eten in
 *   Pacman
 * - zie `assets/handleiding.txt` voor meer informatie
 */

#include <assert.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <ncurses.h>

#include "kleuren.h"
#include "pacman.h"
#include "config.h"
#include "tetris.h"
#include "spel.h"

#define N_VUURWERK 20
#define VUURWERK_STAPPEN 4

const char *VUURWERK[VUURWERK_STAPPEN][5] = {
    {
        "     ",
        "     ",
        "  +  ",
        "     ",
        "     ",
    },
    {
        "     ",
        "  +  ",
        " +#+ ",
        "  +  ",
        "     ",
    },
    {
        "  +  ",
        " ### ",
        "+# #+",
        " ### ",
        "  +  ",
    },
    {
        " ### ",
        "#   #",
        "#   #",
        "#   #",
        " ### ",
    },
};

typedef struct {
    int y;
    int x;
    int leeftijd;
    kleur kleur;
} vuurwerk;

typedef struct {
    pacman *pm_data;
    WINDOW *pm_win;
    int pm_hoogte;
    int pm_breedte;

    tetris *tr_data;
    WINDOW *tr_win;
    int tr_hoogte;
    int tr_breedte;

    int totale_breedte;
    int totale_hoogte;
} spel;

/* Maak een venster in het midden van de terminal
 *
 * hoogte: hoe hoog het venster moet zijn
 * breedte: hoe breed het venster moet zijn
 *
 * Uitvoer: een nieuwe venster
 */
WINDOW *maak_venster(int hoogte, int breedte) {
    int y0 = (LINES - hoogte) / 2;
    int x0 = (COLS - breedte) / 2;
    return newwin(hoogte, breedte, y0, x0);
}

/* Wis het scherm
 *
 * Side effects:
 * - Het scherm wordt gevuld met de achtergrondkleur
 */
void wis_scherm(void) {
    for (int y = 0; y < LINES; y++) {
        move(y, 0);
        for (int x = 0; x < COLS; x++) {
            addch(' ');
        }
    }
}

/* Wacht totdat de speler de ESCAPE toets drukt
 */
void wacht_op_esc(void) {
    while (getch() != 27) {}
}

int ceil_log10(int v) {
    int l = 1;

    while (v >= 10) {
        l++;
        v /= 10;
    }

    return l;
}

/* Teken het win scherm
 *
 * score: de score
 *
 * Side effects:
 * - Het win scherm wordt getekend
 */
void win_scherm(int score) {
    vuurwerk vw[N_VUURWERK];

    for (int i = 0; i < N_VUURWERK; i++) {
        vw[i].y = 4 + (rand() % (LINES - 8));
        vw[i].x = 4 + (rand() % (COLS - 8));
        vw[i].leeftijd = -(rand() % N_VUURWERK);
        vw[i].kleur = K_TETROMINO_0 + (rand() % 7);
    }

    while (1) {
        char toets = getch();
        if (toets == 27 || toets == 'q') {
            break;
        }

        wis_scherm();

        // teken vuurwerk
        for (int i = 0; i < N_VUURWERK; i++) {
            if (vw[i].leeftijd >= 0) {
                kleur_aan(vw[i].kleur);

                for (int y = 0; y < 5; y++) {
                    for (int x = 0; x < 5; x++) {
                        char c = VUURWERK[vw[i].leeftijd][y][x];
                        if (c != ' ') {
                            mvaddch(y + vw[i].y - 2, x*2 + vw[i].x - 4, c | A_DIM);
                            addch(c | A_DIM);
                        }
                    }
                }

                kleur_uit(vw[i].kleur);
            }
        }

        // teken score
        char *tekst = "Je hebt gewonnen!!!";
        mvprintw(2, (COLS - strlen(tekst)) / 2, "%s", tekst);
        mvprintw(3, (COLS - 7 - ceil_log10(score)) / 2, "Score: %d", score);
        refresh();

        // stap vuurwerk
        for (int i = 0; i < N_VUURWERK; i++) {
            vw[i].leeftijd++;

            if (vw[i].leeftijd >= VUURWERK_STAPPEN) {
                vw[i].y = 4 + (rand() % (LINES - 8));
                vw[i].x = 4 + (rand() % (COLS - 8));
                vw[i].leeftijd = -(rand() % (N_VUURWERK * 12 / 10)) - 2;
                vw[i].kleur = K_TETROMINO_0 + (rand() % 7);
            }
        }

        napms(125);
    }
}

/* Teken het verlies scherm
 *
 * score: de score
 *
 * Side effects:
 * - Het verlies scherm wordt getekend
 */
void verlies_scherm(int score) {
    wis_scherm();

    char *tekst = "Je hebt verloren :(";
    mvprintw(2, (COLS - strlen(tekst)) / 2, "%s", tekst);
    mvprintw(3, (COLS - 7 - ceil_log10(score)) / 2, "Score: %d", score);
    refresh();

    wacht_op_esc();
}

/* Maak de spel venster(s)
 *
 * sp: de speldata
 *
 * Side effects:
 * - `pm_win` en `tr_win` worden aangepast
 */
void maak_vensters(spel *sp) {
    if (sp->pm_data != NULL && sp->tr_data != NULL) {
        int totale_breedte = sp->pm_breedte + SPEL_MARGE + sp->tr_breedte;
        int x0 = (COLS - totale_breedte) / 2;

        sp->pm_win = newwin(
            sp->pm_hoogte,
            sp->pm_breedte,
            (LINES - sp->pm_hoogte) / 2,
            x0
        );

        sp->tr_win = newwin(
            sp->tr_hoogte,
            sp->tr_breedte,
            (LINES - sp->tr_hoogte) / 2,
            x0 + sp->pm_breedte + SPEL_MARGE
        );

        sp->totale_breedte = totale_breedte;
        sp->totale_hoogte = MAX(sp->pm_hoogte, sp->tr_hoogte);
    } else if (sp->pm_data != NULL) {
        sp->pm_win = maak_venster(sp->pm_hoogte, sp->pm_breedte);

        sp->totale_breedte = sp->pm_breedte;
        sp->totale_hoogte = sp->pm_hoogte;
    } else if (sp->tr_data != NULL) {
        sp->tr_win = maak_venster(sp->tr_hoogte, sp->tr_breedte);

        sp->totale_breedte = sp->tr_breedte;
        sp->totale_hoogte = sp->tr_hoogte;
    }
}

/* Wis het scherm en maak nieuwe vensters
 *
 * sp: de speldata
 *
 * Side effects:
 * - `pm_win` en `tr_win` worden aangepast
 * - het venster wordt gewist
 */
void na_resize(spel *sp) {
    if (sp->pm_data) {
        delwin(sp->pm_win);
    }

    if (sp->tr_data) {
        delwin(sp->tr_win);
    }

    maak_vensters(sp);
    wis_scherm();
}

/* Zorg dat de terminal groot genoeg is
 *
 * sp: een pointer naar het spel
 *
 * Side effects:
 * - een waarschuwing kan getekend worden
 */
void zorg_voldoende_maat(spel *sp) {
    if (COLS >= sp->totale_breedte && LINES >= sp->totale_hoogte) {
        return;
    }

    while (COLS < sp->totale_breedte || LINES < sp->totale_hoogte) {
        wis_scherm();
        char *bericht = "De terminal is te klein.";
        mvprintw(LINES / 2, (COLS - strlen(bericht)) / 2, "%s", bericht);
        refresh();

        // wacht totdat de terminal groter wordt
        while (getch() != KEY_RESIZE) {}
        na_resize(sp);
    }
}

/* Probeer het spel te maken
 *
 * spel_type: de type van het spel
 * pacman_veld: een pointer naar een rooster dat het pacmanspeelveld bevat
 *
 * Uitvoer: het spel, of NULL als er iets misgaat
 */
spel *maak_spel(spel_type spel_type, rooster *pacman_veld) {
    spel *sp = malloc(sizeof(spel));
    if (sp == NULL) {
        perror("maak_spel");
        return NULL;
    }

    // 1. Maak spellen

    // 1a. Pacman
    int_callback na_verwijder;
    if (spel_type & ST_PACMAN) {
        sp->pm_data = pm_maak(pacman_veld, &sp->pm_hoogte, &sp->pm_breedte);

        if (sp->pm_data == NULL) {
            free(sp);
            return NULL;
        }

        na_verwijder.fn = (int_callback_fn) &pm_verwijderde_regels_cb;
        na_verwijder.userdata = sp->pm_data;
    } else {
        sp->pm_data = NULL;
        sp->pm_win = NULL;
        sp->pm_hoogte = 0;
        sp->pm_breedte = 0;

        na_verwijder.fn = NULL;
        na_verwijder.userdata = NULL;
    }

    // 1b. Tetris
    if (spel_type & ST_TETRIS) {
        sp->tr_data = tr_maak(na_verwijder, &sp->tr_hoogte, &sp->tr_breedte);

        if (sp->tr_data == NULL) {
            if (sp->pm_data) {
                pm_klaar(sp->pm_data);
            }

            free(sp);
            return NULL;
        }
    } else {
        sp->tr_data = NULL;
        sp->tr_win = NULL;
        sp->tr_hoogte = 0;
        sp->tr_breedte = 0;
    }

    // 2. Maak vensters
    maak_vensters(sp);

    return sp;
}

/* Teken het spel
 *
 * sp: een pointer naar het spel
 *
 * Side effects:
 * - alle sub-spellen worden getekend
 */
void teken_spel(spel *sp) {
    if (sp->pm_data) {
        pm_teken(sp->pm_win, sp->pm_data);
        wnoutrefresh(sp->pm_win);
    }

    if (sp->tr_data) {
        tr_teken(sp->tr_win, sp->tr_data);
        wnoutrefresh(sp->tr_win);
    }

    doupdate();
}

/* Kijk wat de totale score is van de spellen
 *
 * sp: een pointer naar het spel
 *
 * Uitvoer: de totale score
 */
int spel_score(const spel *sp) {
    int score = 0;

    if (sp->pm_data) {
        score += pm_score(sp->pm_data);
    }

    if (sp->tr_data) {
        score += tr_score(sp->tr_data);
    }

    return score;
}

/* Teken, reageer op toetsen, en stap het spel
 *
 * sp: een pointer naar het spel
 *
 * Uitvoer: de eindtoestand van het spel
 *
 * Side effects:
 * - het spel verandert
 */
toestand speel(spel *sp) {
    teken_spel(sp);

    clock_t pm_laatste_stap = clock();
    clock_t tr_laatste_stap = clock();

    while (1) {
        zorg_voldoende_maat(sp);

        // 1. Reageer op toetsen
        int toets = getch();
        if (toets != ERR) {
            int gebruikt = (sp->pm_data != NULL && pm_toets(toets, sp->pm_data)) ||
                           (sp->tr_data != NULL && tr_toets(toets, sp->tr_data));

            if (!gebruikt) {
                switch (toets) {
                    case 27: // ESC
                        return NEUTRAAL_KLAAR;
                    case KEY_RESIZE:
                        na_resize(sp);
                        break;
                }
            }
        }

        // 2. Stap, 25 Hz
        clock_t nu = clock();
        clock_t pm_delta = nu - pm_laatste_stap;
        if (sp->pm_data != NULL && pm_delta > CLOCKS_PER_SEC / PM_HERTZ) {
            pm_laatste_stap = nu;
            toestand t = pm_stap(sp->pm_data);
            switch (t) {
                case AAN_HET_SPELEN:
                    break;
                default:
                    return t;
            }
        }

        clock_t tr_delta = nu - tr_laatste_stap;
        if (sp->tr_data != NULL && tr_delta > CLOCKS_PER_SEC / TR_HERTZ) {
            tr_laatste_stap = nu;
            toestand t = tr_stap(sp->tr_data);
            switch (t) {
                case AAN_HET_SPELEN:
                    break;
                default:
                    return t;
            }
        }

        if (sp->pm_data == NULL && sp->tr_data != NULL) {
            if (tr_score(sp->tr_data) >= TR_WIN_SCORE) {
                return GEWONNEN;
            }
        }

        // 3. Teken
        teken_spel(sp);
    }
}

/* Geef alle resources vrij die zijn gealloceerd voor een spel.
 * De spel pointer is na aanroep van deze functie niet meer bruikbaar.
 *
 * sp: een pointer naar het spel
 */
void spel_klaar(spel *sp) {
    if (sp->pm_data) {
        pm_klaar(sp->pm_data);
        delwin(sp->pm_win);
    }

    if (sp->tr_data) {
        tr_klaar(sp->tr_data);
        delwin(sp->tr_win);
    }

    free(sp);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    // 1. Controleer dat er een pacmanbestand is opgegeven op de command line.
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "gebruik: %s allebei|pacman|tetris [PACMAN_ROOSTER]\n", argv[0]);
        return 1;
    }

    spel_type spel_type;
    if (strcmp(argv[1], "allebei") == 0) {
        spel_type = ST_ALLEBEI;
    } else if (strcmp(argv[1], "pacman") == 0) {
        spel_type = ST_PACMAN;
    } else if (strcmp(argv[1], "tetris") == 0) {
        spel_type = ST_TETRIS;
    } else {
        fprintf(stderr, "gebruik: %s allebei|pacman|tetris [PACMAN_ROOSTER]\n", argv[0]);
        return 1;
    }

    // 2. Open het pacmanbestand en lees het rooster.
    rooster *pacman_veld = NULL;
    if (spel_type & ST_PACMAN) {
        if (argc == 2) { // pacman heeft wel een roosterbestand nodig
            fprintf(stderr, "gebruik: %s %s PACMAN_ROOSTER\n", argv[0], argv[1]);
            return 1;
        }

        FILE *fh = fopen(argv[2], "r");
        if (fh == NULL) {
            perror("main");
            return 1;
        }
        pacman_veld = rooster_lees(fh);
        fclose(fh);

        // 3. Bepaal of het lezen van het rooster is gelukt.
        if (pacman_veld == NULL) {
            fprintf(stderr, "Kan rooster niet maken.\n");
            return 1;
        }
    } else if (argc == 3) { // tetris heeft geen roosterbestand nodig
        fprintf(stderr, "gebruik: %s %s\n", argv[0], argv[1]);
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
    spel *spel = maak_spel(spel_type, pacman_veld);
    if (spel == NULL) {
        fprintf(stderr, "Kan spel niet maken.\n");
        return 1;
    }

    // 6. Speel het spel.
    toestand eindtoestand = speel(spel);
    int score = spel_score(spel);
    spel_klaar(spel);

    // 7. Toon win- of verlies scherm
    switch (eindtoestand) {
        case GEWONNEN:
            win_scherm(score);
            break;
        case VERLOREN:
            verlies_scherm(score);
            break;
        default:
            break;
    }

    // 7. Sluit af.
    endwin();
    return 0;
}
