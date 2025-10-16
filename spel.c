/*
 * Naam: S.F.L. Wagenaar
 * UvAnetID: 16409752
 * Studie: BSc Informatica
 *
 * spel.c
 * - roep aan met een bestand wat een rechthoekige doolhof heeft
 *   `./spel assets/pacman.txt`
 * - een combinatie van Pacman en Tetris
 * - de speler kan 8 seconden lang spoken eten na een rij te verwijderen in Tetris
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

typedef struct {
    pacman *pm_data;
#if PACMAN
    WINDOW *pm_win;
    int pm_hoogte;
    int pm_breedte;
#endif

    tetris *tr_data;
#if TETRIS
    WINDOW *tr_win;
    int tr_hoogte;
    int tr_breedte;
#endif

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

/* Teken het win scherm
 *
 * Side effects:
 * - Het win scherm wordt getekend
 */
void win_scherm(void) {
    wis_scherm();

    char *tekst = "Je hebt gewonnen!!!";
    mvprintw(2, (COLS - strlen(tekst)) / 2, "%s", tekst);
    refresh();

    wacht_op_esc();
}

/* Teken het verlies scherm
 *
 * Side effects:
 * - Het verlies scherm wordt getekend
 */
void verlies_scherm(void) {
    wis_scherm();

    char *tekst = "Je hebt verloren :(";
    mvprintw(2, (COLS - strlen(tekst)) / 2, "%s", tekst);
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
#if PACMAN && TETRIS
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
#elif PACMAN
    sp->pm_win = maak_venster(sp->pm_hoogte, sp->pm_breedte);
    sp->totale_breedte = sp->pm_breedte;
    sp->totale_hoogte = sp->pm_hoogte;
#elif TETRIS
    sp->tr_win = maak_venster(sp->tr_hoogte, sp->tr_breedte);
    sp->totale_breedte = sp->tr_breedte;
    sp->totale_hoogte = sp->tr_hoogte;
#endif
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
#if PACMAN
    delwin(sp->pm_win);
#endif

#if TETRIS
    delwin(sp->tr_win);
#endif

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
 * pacman_veld: een pointer naar een rooster dat het pacmanspeelveld bevat
 *
 * Uitvoer: het spel, of NULL als er iets misgaat
 */
spel *maak_spel(rooster *pacman_veld) {
    spel *sp = malloc(sizeof(spel));
    if (sp == NULL) {
        perror("maak_spel");
        return NULL;
    }

    // 1. Maak spellen

    // we maken altijd het pacmanspel (om bugs zoveel mogelijk te vangen),
    // maar in de debugmodus maken we het venster niet.
    // hierdoor kunnen we tetris testen zonder ons zorgen te hoeven maken over twee vensters
    int pm_hoogte, pm_breedte;
    sp->pm_data = pm_maak(pacman_veld, &pm_hoogte, &pm_breedte);

    if (sp->pm_data == NULL) {
        free(sp);
        return NULL;
    }

    // ditto het tetrisspel
#if PACMAN
    int_callback na_verwijder = {
        .fn = (int_callback_fn) &pm_verwijderde_regels_cb,
        .userdata = sp->pm_data,
    };
#else
    int_callback na_verwijder = {
        .fn = NULL,
        .userdata = NULL,
    };
#endif

    int tr_hoogte, tr_breedte;
    sp->tr_data = tr_maak(na_verwijder, &tr_hoogte, &tr_breedte);

    if (sp->tr_data == NULL) {
#if PACMAN
        pm_klaar(sp->pm_data);
#endif
        free(sp);
        return NULL;
    }

    // 2. Maak vensters

#if PACMAN
    sp->pm_hoogte = pm_hoogte;
    sp->pm_breedte = pm_breedte;
#endif

#if TETRIS
    sp->tr_hoogte = tr_hoogte;
    sp->tr_breedte = tr_breedte;
#endif

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
#if PACMAN
    pm_teken(sp->pm_win, sp->pm_data);
    wnoutrefresh(sp->pm_win);
#else
    mvprintw(0, 0, "DEBUG: Pacman is niet actief");
#endif

#if TETRIS
    tr_teken(sp->tr_win, sp->tr_data);
    wnoutrefresh(sp->tr_win);
#else
    mvprintw(1, 0, "DEBUG: Tetris is niet actief");
#endif

    doupdate();
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

#if PACMAN
    clock_t pm_laatste_stap = clock();
#endif

#if TETRIS
    clock_t tr_laatste_stap = clock();
#endif

    while (1) {
        zorg_voldoende_maat(sp);

        // 1. Reageer op toetsen
        int toets = getch();
        if (toets != ERR) {
#if PACMAN && TETRIS
            int gebruikt = pm_toets(toets, sp->pm_data) ||
                           tr_toets(toets, sp->tr_data);
#elif PACMAN
            int gebruikt = pm_toets(toets, sp->pm_data);
#elif TETRIS
            int gebruikt = tr_toets(toets, sp->tr_data);
#else
            int gebruikt = 0;
#endif

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
#if PACMAN
        clock_t pm_delta = nu - pm_laatste_stap;
        if (pm_delta > CLOCKS_PER_SEC / PM_HERTZ) {
            pm_laatste_stap = nu;
            toestand t = pm_stap(sp->pm_data);
            switch (t) {
                case AAN_HET_SPELEN:
                    break;
                default:
                    return t;
            }
        }
#endif

#if TETRIS
        clock_t tr_delta = nu - tr_laatste_stap;
        if (tr_delta > CLOCKS_PER_SEC / TR_HERTZ) {
            tr_laatste_stap = nu;
            toestand t = tr_stap(sp->tr_data);
            switch (t) {
                case AAN_HET_SPELEN:
                    break;
                default:
                    return t;
            }
        }
#endif

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
    pm_klaar(sp->pm_data);
#if PACMAN
    delwin(sp->pm_win);
#endif

    tr_klaar(sp->tr_data);
#if TETRIS
    delwin(sp->tr_win);
#endif

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
    toestand eindtoestand = speel(spel);
    spel_klaar(spel);

    // 7. Toon win- of verlies scherm
    switch (eindtoestand) {
        case GEWONNEN:
            win_scherm();
            break;
        case VERLOREN:
            verlies_scherm();
            break;
        default:
            break;
    }

    // 7. Sluit af.
    endwin();
    return 0;
}
