#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "tetris.h"
#include "config.h"
#include "kleuren.h"
#include "spel.h"

#define BOVEN_MARGE 4
#define LEEG -1
#define MUUR -2

#define BEGIN_X (TR_BREEDTE / 2 - 2)
#define BEGIN_Y (-1)

// twee verschillende types, want c11 gedraagt vreemd met
// het casten van `int[][]` naar `const int[][]`
typedef int kuil[TR_HOOGTE][TR_BREEDTE];
typedef int (*kuil_ptr)[TR_BREEDTE];
typedef const int (*const_kuil_ptr)[TR_BREEDTE];

const char *TETROMINOS[7][4] = {
    {
        "    ",
        "    ",
        "####",
        "    ",
    },
    {
        "    ",
        " ## ",
        " ## ",
        "    ",
    },
    {
        "    ",
        "  ##",
        " ## ",
        "    ",
    },
    {
        "    ",
        "##  ",
        " ## ",
        "    ",
    },
    {
        "    ",
        " #  ",
        "### ",
        "    ",
    },
    {
        "    ",
        "### ",
        "  # ",
        "    ",
    },
    {
        "    ",
        " ###",
        " #  ",
        "    ",
    },
};

// de x-offset van tetrominos in het preview venster
const int TETROMINO_PREVIEW_X[7] = { 0, 0, -1, 1, 1, 1, -1 };

typedef struct {
    int type;
    int y;
    int x;
    int rot;
} tetromino;

struct tetris_data {
    kuil kuil;
    int volgende_type;
    tetromino tet;
    int score;

    int_callback na_verwijder;
    int hoogte;
    int breedte;
};

/* Kijk of een tetromino op een gegeven positie staat
 *
 * tet: de tetromino
 * y: de y-positie
 * x: de x-positie
 *
 * Uitvoer: 1 als de tetromino op (x, y) staat, ander 0
 */
static int tet_op(const tetromino *tet, int y, int x) {
    y -= tet->y;
    x -= tet->x;

    if (x < 0 || x >= 4 || y < 0 || y >= 4) {
        return 0;
    }

    /* draai van:
     * +----
     * |..#.
     * |....
     * |....
     * |....
     *
     * naar:
     * +----
     * |....
     * |....
     * |...#
     * |....
     */
    for (int i = 0; i < tet->rot; i++) {
        int tmp = x;
        x = 3 - y;
        y = tmp;
    }

    return TETROMINOS[tet->type][y][x] == '#';
}

/* Kijk wat voor blok op een plaats in de kuil zit
 *
 * kuil: de kuil
 * y: de y-positie om op te kijken
 * x: de x-positie om op te kijken
 *
 * Uitvoer: een tetromino type, LEEG, of MUUR
 */
static int kijk_kuil(const_kuil_ptr kuil, int y, int x) {
    if (y < 0) {
        return LEEG;
    }

    if (x < 0 || x >= TR_BREEDTE || y >= TR_HOOGTE) {
        return MUUR;
    }

    return kuil[y][x];
}

/****************/
/* Stapfuncties */
/****************/

/* Plaat een blok op een plaats in de kuil
 *
 * kuil: de kuil
 * y: de y-positie om op te plaatsen
 * x: de x-positie om op te plaatsen
 * type: de type van het blok
 *
 * Side effects:
 * - de kuil wordt aangepast, als (x, y) binnen de kuil ligt
 */
static void zet_kuil(kuil_ptr kuil, int y, int x, int type) {
    if (y < 0 || x < 0 || x >= TR_BREEDTE || y >= TR_HOOGTE) {
        return;
    }

    kuil[y][x] = type;
}

/* Kijk of een tetromino in de kuil past
 *
 * kuil: de kuil
 * tet: de tetromino om te testen
 *
 * Uitvoer: 1 als de tetromino past, anders 0
 */
static int tet_past(const_kuil_ptr kuil, const tetromino *tet) {
    for (int y = tet->y; y < tet->y + 4; y++) {
        for (int x = tet->x; x < tet->x + 4; x++) {
            if (tet_op(tet, y, x) && kijk_kuil(kuil, y, x) != LEEG) {
                return 0;
            }
        }
    }

    return 1;
}

/* Probeer een tetromino te bewegen
 *
 * kuil: de kuil
 * tet: de tetromino om te bewegen
 * dy: de verandering van de y-positie
 * dx: de verandering van de x-positie
 * drot: de verandering van de rotatie
 *
 * Side effects:
 * - de tetromino verandert van plaats/rotatie als ie in de nieuwe staat nog past
 */
static int probeer_beweging(const_kuil_ptr kuil, tetromino *tet, int dy, int dx, int drot) {
    tet->y += dy;
    tet->x += dx;
    tet->rot = (tet->rot + drot) % 4;

    if (tet_past(kuil, tet)) {
        return 1;
    } else {
        tet->y -= dy;
        tet->x -= dx;
        tet->rot = (tet->rot + 4 - drot) % 4;
        return 0;
    }
}

/* Kijk of een regel vol is
 *
 * kuil: de kuil
 * y: de regel om te bekijken
 *
 * Uitvoer: 1 als de regel vol is, anders 0
 */
static int is_regel_vol(const_kuil_ptr kuil, int y) {
    for (int x = 0; x < TR_BREEDTE; x++) {
        if (kijk_kuil(kuil, y, x) == LEEG) {
            return 0;
        }
    }

    return 1;
}

/* Verwijder volle regels van de kuil
 *
 * kuil: de kuil om regels van de verwijderen
 *
 * Uitvoer: het aantal verwijderde regels
 *
 * Side effects:
 * - volle regels worden verwijderd
 */
static int verwijder_volle_regels(kuil_ptr kuil) {
    int n =0;

    int schrijf = TR_HOOGTE - 1;

    for (int lees = TR_HOOGTE - 1; lees >= 0; lees--) {
        for (int x = 0; x < TR_BREEDTE; x++) {
            kuil[schrijf][x] = kuil[lees][x];
        }

        if (is_regel_vol((const_kuil_ptr) kuil, lees)) {
            n++;
        } else {
            schrijf--;
        }
    }

    return n;
}

/* Plaats een tetromino in de kuil
 *
 * kuil: de kuil
 * tet: de tetromino om te plaatsen
 *
 * Uitvoer: het aantal verwijderde regels
 *
 * Side effects:
 * - de kuil wordt aangepast
 */
static int plaats_tet(kuil_ptr kuil, const tetromino *tet) {
    for (int y = tet->y; y < tet->y + 4; y++) {
        for (int x = tet->x; x <tet->x + 4; x++) {
            if (tet_op(tet, y, x)) {
                zet_kuil(kuil, y, x, tet->type);
            }
        }
    }

    return verwijder_volle_regels(kuil);
}

/* Ga naar de volgende tetromino en genereer een nieuwe
 *
 * data: de speldata
 *
 * Side effects:
 * - de huidige tetromino verandert van type en gaat naar de bovenkant
 * - de volgende tetromino wordt opnieuw gekozen
 */
static void gebruik_volgende_tet(tetris *data) {
    int volgende = rand() % 7;
    while (
#if TR_UNIEK_PERIODE >= 1
        volgende == data->volgende_type ||
#endif
#if TR_UNIEK_PERIODE >= 2
        volgende == data->tet.type ||
#endif
        0
    ) {
        volgende = rand() % 7;
    }

    data->tet.type = data->volgende_type;
    data->tet.y = BEGIN_Y;
    data->tet.x = BEGIN_X;
    data->tet.rot = 0;

    data->volgende_type = volgende;
}

/*****************/
/* Tekenfuncties */
/*****************/

/* Teken een doos om een rechthoek heen.
 *
 * win: het venster om op te tekenen
 * y0: de y coördinaat van de linkerbovenhoek van de omringde ruimte
 * x0: de x coördinaat van de linkerbovenhoek van de omringde ruimte
 * hoogte: de hoogte van de omringde ruimte
 * breedte: de breedte van de omringde ruimte
 * kleur: de kleur van de doos
 *
 * Side effects:
 * - er wordt een doos getekend op het venster
 */
static void teken_doos_om(WINDOW *win, int y0, int x0, int hoogte, int breedte, kleur kleur) {
    wkleur_aan(win, kleur);

    wmove(win, y0 - 1, x0 - 1);
    waddch(win, ACS_ULCORNER);
    for (int x = 0; x < breedte; x++) {
        waddch(win, ACS_HLINE);
    }
    waddch(win, ACS_URCORNER);

    for (int y = 0; y < hoogte; y++) {
        mvwaddch(win, y0 + y, x0 - 1, ACS_VLINE);
        mvwaddch(win, y0 + y, x0 + breedte, ACS_VLINE);
    }

    wmove(win, y0 + hoogte, x0 - 1);
    waddch(win, ACS_LLCORNER);
    for (int x = 0; x < breedte; x++) {
        waddch(win, ACS_HLINE);
    }
    waddch(win, ACS_LRCORNER);

    wkleur_uit(win, kleur);
}

/* Teken een pixel op de gegeven venster-coördinaten
 *
 * win: het venster om op te tekenen
 * y: de y-coördinaat
 * x: de x-coördinaat
 * kleur: de kleur van de pixel
 *
 * Side effects:
 * - een pixel wordt op het venster getekend
 */
static void teken_pixel_rauw(WINDOW *win, int y, int x, kleur kleur) {
    wkleur_aan(win, kleur);
    mvwaddch(win, y, x, ' ');
    waddch(win, ' ');
    wkleur_uit(win, kleur);
}

/* Teken een pixel op de gegeven kuil-coördinaten
 *
 * win: het venster om op te tekenen
 * y: de y-coördinaat
 * x: de x-coördinaat
 * kleur: de kleur van de pixel
 *
 * Side effects:
 * - een pixel wordt op het venster getekend
 */
static void teken_pixel(WINDOW *win, int y, int x, kleur kleur) {
    int y0 = 1 + BOVEN_MARGE;
    int x0 = 2;

    teken_pixel_rauw(win, y0 + y, x0 + 2*x, kleur);
}

/* Teken de muren in inhoud van de kuil
 *
 * win: het venster om op te tekenen
 * kuil: de kuil om te tekenen
 *
 * Side effects:
 * - de kuil wordt op het venster getekend
 */
static void teken_kuil(WINDOW *win, const_kuil_ptr kuil) {
    wkleur_aan(win, K_TR_MUUR);
    for (int y = 1 + BOVEN_MARGE; y < 1 + BOVEN_MARGE + TR_HOOGTE; y++) {
        mvwaddch(win, y, 1, ']');
        mvwaddch(win, y, 2 + TR_BREEDTE*2, '[');
    }
    wmove(win, 1 + BOVEN_MARGE + TR_HOOGTE, 1);
    for (int x = 0; x < 2 + TR_BREEDTE*2; x++) {
        waddch(win, '#');
    }
    wkleur_uit(win, K_TR_MUUR);

    for (int y = -2; y < TR_HOOGTE; y++) {
        for (int x = 0; x < TR_BREEDTE; x++) {
            int blok = kijk_kuil(kuil, y, x);
            if (blok <= LEEG) {
                kleur achtergrond = y < 0 ? K_DEFAULT
                                          : x % 2 == 0 ? K_TR_ACHTERGROND
                                                       : K_TR_ACHTERGROND_ALT;
                teken_pixel(win, y, x, achtergrond);
            } else {
                teken_pixel(win, y, x, K_TETROMINO_0 + blok);
            }
        }
    }
}

/* Teken het vallende tetromino
 *
 * win: het venster om op te tekenen
 * tet: de tetromino
 *
 * Side effects:
 * - een tetromino wordt op het venster getekend
 */
static void teken_tetromino(WINDOW *win, const tetromino *tet) {
    for (int y = tet->y; y < tet->y + 4; y++) {
        for (int x = tet->x; x < tet->x + 4; x++) {
            if (tet_op(tet, y, x)) {
                teken_pixel(win, y, x, K_TETROMINO_0 + tet->type);
            }
        }
    }
}

/* Teken een venstertje met de volgende tetromino
 *
 * win: het venster om op te tekenen
 * type: de type van de volgende tetromino
 *
 * Side effects:
 * - de volgende tetromino wordt op het venster getekend
 */
static void teken_volgende_tetromino(WINDOW *win, int type) {
    int y0 = 1 + BOVEN_MARGE;
    int x0 = 6 + TR_BREEDTE * 2;

    tetromino volgende = {
        .type = type,
        .y = 0,
        .x = 0,
        .rot = 0,
    };

    // wis het vakje
    wkleur_aan(win, K_TR_ACHTERGROND);
    for (int y = 0; y < 4; y++) {
        mvwprintw(win, y0 + y, x0, "        ");
    }
    wkleur_uit(win, K_TR_ACHTERGROND);

    // maak een venstertje
    teken_doos_om(win, y0, x0, 4, 8, K_TR_VENSTER);

    // teken de tetromino
    int schuiving = TETROMINO_PREVIEW_X[type];
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (tet_op(&volgende, y, x)) {
                teken_pixel_rauw(win, y0 + y, x0 + 2*x + schuiving, K_TETROMINO_0 + type);
            }
        }
    }
}

/* Teken de score
 *
 * win: het venster om op te tekenen
 * breedte: de breedte van het venster
 * score: de score
 *
 * Side effects:
 * - de score wordt op het venster getekend
 */
static void teken_score(WINDOW *win, int breedte, int score) {
    wmove(win, 1, (breedte - 7 - ceil_log10(score)) / 2);
    wprintw(win, "Score: %d", score);
}

tetris *tr_maak(int_callback na_verwijder, int *hoogte, int *breedte) {
    tetris *data = malloc(sizeof(tetris));
    if (data == NULL) {
        perror("tr_maak");
        return NULL;
    }

    // maak kuil leeg
    for (int y = 0; y < TR_HOOGTE; y++) {
        for (int x = 0; x < TR_BREEDTE; x++) {
            data->kuil[y][x] = LEEG;
        }
    }

    // venster, kuil, marge
    data->hoogte = *hoogte  = 2 + TR_HOOGTE + 1 + BOVEN_MARGE;
    // venster, kuil, marge, volgende stuk, marge
    data->breedte = *breedte = 2 + (TR_BREEDTE*2 + 2) + 4 + 8 + 1;

    // zet beginwaardes
    data->tet.type = rand() % 7;
    data->tet.y    = BEGIN_Y;
    data->tet.x    = BEGIN_X;
    data->tet.rot  = 0;

    data->volgende_type = rand() % 7;
#if TR_UNIEK_PERIODE > 0
    while (data->volgende_type == data->tet.type) {
        data->volgende_type = rand() % 7;
    }
#endif

    data->score = 0;

    data->na_verwijder = na_verwijder;

    return data;
}

void tr_klaar(tetris *data) {
    free(data);
}

int tr_toets(int toets, tetris *data) {
    switch (toets) {
        case ',':
        case KEY_DOWN: // draai naar links
            probeer_beweging((const_kuil_ptr) data->kuil, &data->tet, 0, 0, 1);
            break;
        case '.':
        case KEY_UP: // draai naar rechts
            probeer_beweging((const_kuil_ptr) data->kuil, &data->tet, 0, 0, 3);
            break;

        case KEY_LEFT: // beweeg naar links
            probeer_beweging((const_kuil_ptr) data->kuil, &data->tet, 0, -1, 0);
            break;
        case KEY_RIGHT: // beweeg naar rechts
            probeer_beweging((const_kuil_ptr) data->kuil, &data->tet, 0, 1, 0);
            break;

        case KEY_SLEFT: // spring naar links
            while (1) {
                if (!probeer_beweging((const_kuil_ptr) data->kuil, &data->tet, 0, -1, 0)) {
                    break;
                }
            }
            break;
        case KEY_SRIGHT: // spring naar rechts
            while (1) {
                if (!probeer_beweging((const_kuil_ptr) data->kuil, &data->tet, 0, 1, 0)) {
                    break;
                }
            }
            break;

        case '\n': // spring naar beneden
            while (1) {
                if (!probeer_beweging((const_kuil_ptr) data->kuil, &data->tet, 1, 0, 0)) {
                    break;
                }

                data->score++;
            }
            break;
    }

    return 0;
}

toestand tr_stap(tetris *data) {
    if (!probeer_beweging((const_kuil_ptr) data->kuil, &data->tet, 1, 0, 0)) {
        int verwijderde_regels = plaats_tet(data->kuil, &data->tet);

        if (data->na_verwijder.fn != NULL) {
            data->na_verwijder.fn(verwijderde_regels, data->na_verwijder.userdata);
        }

        data->score += TR_REGEL_PUNTEN * verwijderde_regels;

        gebruik_volgende_tet(data);
        if (!tet_past((const_kuil_ptr) data->kuil, &data->tet)) {
            return VERLOREN;
        }
    }

    return AAN_HET_SPELEN;
}

void tr_teken(WINDOW *win, const tetris *data) {
    wkleur_uit(win, K_TR_VENSTER);
    box(win, 0, 0);
    wkleur_uit(win, K_TR_VENSTER);

    teken_kuil(win, data->kuil);
    teken_tetromino(win, &data->tet);
    teken_volgende_tetromino(win, data->volgende_type);
    teken_score(win, data->breedte, data->score);
}

int tr_score(const tetris *data) {
    return data->score;
}
