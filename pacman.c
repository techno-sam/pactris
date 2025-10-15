#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "pacman.h"
#include "kleuren.h"
#include "rooster.h"

#define MAX_LEVENS 3
#define SCORE_HOOGTE 3

#define ALLE_RICHTINGEN(var) (richting var = RECHTS; var <= OMHOOG; var++)
#define SPIEGEL_RICHTING(rot) (rot == GEEN ? GEEN : (rot + 2) % 4)

// data tabellen, geïndexeerd met een richting
const int  DX[]         = {   1,   0,  -1,   0 };
const int  DY[]         = {   0,   1,   0,  -1 };
const char SPELER_STD[] = { '<', '^', '>', 'v' };
const char SPELER_ALT[] = { '-', '|', '-', '|' };

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

typedef void(*doel_kiezer)(const pacman *data, int *doel_x, int *doel_y);

typedef struct {
    entity ent;
    doel_kiezer doel;
    kleur kleur;
    // spoken met een positieve huis_arrest waarde zitten vast in het spookhuis.
    // als de speler een stukje voedsel eet wordt de huis_arrest van het spook met de hoogste
    // prioriteit verminderd, in de volgorde Blinky, Pinky, Inky, Clyde
    int huis_arrest;
} spook;

typedef struct {
    entity speler;
    entity blinky;
    entity pinky;
    entity inky;
    entity clyde;
} originele_plaatsen;

struct pacman_data {
    rooster *veld;
    int breedte;
    int hoogte;
    int stappen;

    int levens;
    int begin_voedsel;
    int gegeten_voedsel;

    int dood_animatie;

    originele_plaatsen start;

    richting doel_rot;
    entity speler;

    spook blinky;
    spook pinky;
    spook inky;
    spook clyde;
};

// Spook doelen zijn van
// https://www.gamedeveloper.com/design/the-pac-man-dossier#Chapter%204:%20Meet%20the%20Ghosts

void doel_blinky(const pacman *data, int *doel_x, int *doel_y) {
    // Blinky volgt de speler
    *doel_x = data->speler.x;
    *doel_y = data->speler.y;
}

void doel_pinky(const pacman *data, int *doel_x, int *doel_y) {
    // Pinky probeert voor de speler te komen
    *doel_x = data->speler.x + 4 * DX[data->speler.rot];
    *doel_y = data->speler.y + 4 * DY[data->speler.rot];
}

// NOTE: hangt af van blinky.ent.{x,y}, dus Inky moet voor Blinky bewegen
void doel_inky(const pacman *data, int *doel_x, int *doel_y) {
    // Inky loopt weg van de speler als Blinky ver van de speler is,
    // anders volgt ie de speler
    int tmp_x = data->speler.x + 2 * DX[data->speler.rot];
    int tmp_y = data->speler.y + 2 * DY[data->speler.rot];

    *doel_x = tmp_x + (tmp_x - data->blinky.ent.x);
    *doel_y = tmp_y + (tmp_y - data->blinky.ent.y);
}

void doel_clyde(const pacman *data, int *doel_x, int *doel_y) {
    // Clyde wil naar de linkeronderhoek als ie binnen 8 stappen van de speler is,
    // anders volgt ie de speler

    int dx = data->speler.x - data->clyde.ent.x;
    int dy = data->speler.y - data->clyde.ent.y;
    int d = dx*dx + dy*dy;

    if (d < 8*8) {
        *doel_x = 0;
        *doel_y = data->hoogte + 1;
    } else {
        *doel_x = data->speler.x;
        *doel_y = data->speler.y;
    }
}

/* Maak een spook met een aantal default waardes
 *
 * doel: functie om de pathfinding doel van het spook te bepalen
 * kleur: de kleur van het spook
 *
 * Uitvoer: een spook op (0, 0) dat omhoog wijst en geen huis arrest heeft
 */
spook maak_spook(doel_kiezer doel, kleur kleur) {
    spook sp = {
        .ent = { .x = 0, .y = 0, .rot = OMHOOG },
        .huis_arrest = -1,
        .doel = doel,
        .kleur = kleur
    };
    return sp;
}

/* Herstel alle spoken en de speler naar hun startposities
 *
 * data: de speldata
 *
 * Side effects:
 * - de posities van de spoken en speler worden veranderd
 * - de huis_arrest waardes van de spoken worden veranderd
 */
void ga_naar_start(pacman *data) {
    data->speler = data->start.speler;
    data->blinky.ent = data->start.blinky;
    data->pinky.ent = data->start.pinky;
    data->inky.ent = data->start.inky;
    data->clyde.ent = data->start.clyde;

    data->blinky.huis_arrest = -1;
    data->pinky.huis_arrest  =  0;
    data->inky.huis_arrest   = 30;
    data->clyde.huis_arrest  = 60;
}

/* Plaats een entity op z'n startpositie
 *
 * veld: het veld om te doorzoeken
 * c: de karakter die de startpositie aangeeft
 * ent: de entity om te plaatsen
 * rot: de startrichting van de entity
 *
 * Uitvoer: 1 als een plaats beschikbaar was, anders 0
 *
 * Side effects:
 * - de richting van de entity wordt gezet
 * - de positie van de entity wordt misschien gezet
 * - de startpositiekarakter wordt verwijderd
 */
int zoek_plaats(rooster *veld, char c, entity *ent, richting rot) {
    ent->rot = rot;

    rooster_zoek(veld, c, &ent->x, &ent->y);
    if (ent->x == -1) {
        return 0;
    } else {
        rooster_plaats(veld, ent->x, ent->y, ' ');
        return 1;
    }
}

/* Kijk of een vakje bewandelbaar is
 *
 * c: de inhoud van het vakje
 *
 * Uitvoer: 1 als het vakje bewandelbaar is, anders 0
 */
int bewandelbaar(char c) {
    return c == ' ' || c == '.';
}

/****************/
/* Stapfuncties */
/****************/

/* Kijk of het mogelijk is om een vakje vooruit te bewegen in een bepaalde richting
 *
 * veld: het veld om door te lopen
 * ent: de entity (speler of spook) dat moet lopen
 * rot: de richting om te proberen
 *
 * Uitvoer: 1 als de stap mogelijk is, anders 0
 */
int kan_bewegen(const rooster *veld, const entity *ent, richting rot) {
    int x1 = ent->x + DX[rot];
    int y1 = ent->y + DY[rot];

    char c = rooster_kijk(veld, x1, y1);

    return bewandelbaar(c);
}

/* Probeer een entity een vakje vooruit te bewegen
 *
 * veld: het veld om door te lopen
 * ent: de entity (speler of spook) dat moet lopen
 *
 * Uitvoer: de karakter waar `ent` nu op staat, of '\0' als geen stap mogelijk was
 *
 * Side effects:
 * - verplaatst `ent`
 */
char entity_loop(const rooster *veld, entity *ent) {
    int x1 = ent->x + DX[ent->rot];
    int y1 = ent->y + DY[ent->rot];

    char c = rooster_kijk(veld, x1, y1);

    if (bewandelbaar(c)) {
        ent->x = x1;
        ent->y = y1;
        return c;
    } else {
        return '\0';
    }
}

/* Probeer de wandelrichting van de speler te veranderen
 *
 * data: de speldata
 *
 * Side effects:
 * - de wandelrichting van de speler kan veranderen
 * - de doelrichting kan worden gewist
 */
void verander_speler_richting(pacman *data) {
    if (data->doel_rot == GEEN) {
        return;
    }

    if (data->doel_rot == data->speler.rot) {
        data->doel_rot = GEEN;
        return;
    }

    int x1 = data->speler.x + DX[data->doel_rot];
    int y1 = data->speler.y + DY[data->doel_rot];
    char c = rooster_kijk(data->veld, x1, y1);

    if (bewandelbaar(c)) {
        data->speler.rot = data->doel_rot;
        data->doel_rot = GEEN;
    }
}

/* Probeer de huis_arrest van het spook met de hoogste prioriteit te verminderen
 *
 * data: de speldata
 *
 * Side effects:
 * - de huis_arrest van maximaal een spook wordt verminderd
 */
void verminder_huis_arrest(pacman *data) {
    spook *spoken[] = {
        &data->blinky,
        &data->pinky,
        &data->inky,
        &data->clyde,
    };

    for (int i = 0; i < 4; i++) {
        spook *sp = spoken[i];
        if (sp->huis_arrest > 0) {
            sp->huis_arrest--;
            break;
        }
    }
}

/* Kijk of de speler door een spook is gegeten
 *
 * speler: de speler
 * spook: het spook
 *
 * Uitvoer: 1 als de speler en het spook in hetzelfde vakje zitten, anders 0
 */
int is_gegeten(const entity *speler, const spook *spook) {
    return speler->x == spook->ent.x && speler->y == spook->ent.y;
}

/* Kijk of de speler door een van de spoken is gegeten
 *
 * data: de speldata
 *
 * Uitvoer: 1 als de speler en een spook in hetzelfde vakje zitten, ander 0
 */
int is_gegeten_alle_spoken(const pacman *data) {
    return is_gegeten(&data->speler, &data->blinky) ||
           is_gegeten(&data->speler, &data->pinky) ||
           is_gegeten(&data->speler, &data->inky) ||
           is_gegeten(&data->speler, &data->clyde);
}

/* Simuleer een stap van een spook
 *
 * data: de speldata
 * spook: het spook om te simuleren
 *
 * Side effects:
 * - het spook vermindert z'n huis_arrest of beweegt
 */
void stap_spook(const pacman *data, spook *spook) {
    if (spook->huis_arrest == 0) {
        spook->huis_arrest = -1;

        // vind de deur en ga er op staan
        // (doordat het spook omhoog wijst zal ie nooit weer naar binnen lopen)
        spook->ent.rot = OMHOOG;
        rooster_zoek(data->veld, '+', &spook->ent.x, &spook->ent.y);
        return;
    } else if (spook->huis_arrest >= 0) {
        return;
    }

    // vind een doel
    int doel_x, doel_y;
    spook->doel(data, &doel_x, &doel_y);

    // welke richting moeten we in om bij onze doel te komen?
    int beste_score = -1;
    richting beste_rot = spook->ent.rot;
    for ALLE_RICHTINGEN(rot) {
        if (!kan_bewegen(data->veld, &spook->ent, rot)) {
            continue;
        }

        // hoe ver zouden we van onze doel zijn?
        int dx = spook->ent.x + DX[rot] - doel_x;
        int dy = spook->ent.y + DY[rot] - doel_y;
        int score = dx*dx + dy*dy;

        // ga alleen achteruit als er geen andere keuzes zijn
        if (SPIEGEL_RICHTING(rot) == spook->ent.rot) {
            score = INT_MAX;
        }

        if (beste_score == -1 || score < beste_score) {
            beste_score = score;
            beste_rot = rot;
        }
    }
    spook->ent.rot = beste_rot;

    entity_loop(data->veld, &spook->ent);
}

/* Simuleer een stap van een speler
 *
 * data: de speldata
 *
 * Side effects:
 * - de speler beweegt of gaat dood
 * - voedsel wordt gegeten
 */
void stap_speler(pacman *data) {
    // zodat pacman niet door een spook heen kan lopen
    // (`>@` -> `@>` zou anders mogelijk zijn)
    int gegeten = is_gegeten_alle_spoken(data);

    // pacman loopt ~2x zo snel als de spoken
    if (data->stappen % 5 == 0) {
        char c = entity_loop(data->veld, &data->speler);
        if (c == '.') {
            rooster_plaats(data->veld, data->speler.x, data->speler.y, ' ');
            data->gegeten_voedsel++;
            verminder_huis_arrest(data);
        }
    }

    if (gegeten || is_gegeten_alle_spoken(data)) {
        data->dood_animatie = 12;
        data->levens--;
    }
}

/*****************/
/* Tekenfuncties */
/*****************/

/* Teken de score
 *
 * win: het venster om op te tekenen
 * data: de speldata
 *
 * Side effects:
 * - tekst wordt naar het venster geschreven
 */
void teken_score(WINDOW *win, const pacman *data) {
    mvwprintw(win, 1, 1, "Score: %d / %d", data->gegeten_voedsel, data->begin_voedsel);

    // levens/hartjes
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
}

/* Teken het speelveld
 *
 * win: het venster om op te tekenen
 * rp: de speelveldrooster
 *
 * Side effects:
 * - tekst wordt naar het venster geschreven
 */
void teken_rooster(WINDOW *win, const rooster *rp) {
    int breedte = rooster_breedte(rp);
    int hoogte = rooster_hoogte(rp);

    for (int y = 0; y < hoogte; y++) {
        wmove(win, y + 1 + SCORE_HOOGTE, 1);

        for (int x = 0; x < breedte; x++) {
            char c0 = rooster_kijk(rp, x, y);
            chtype c = c0;

            chtype huis_muur = A_BOLD | COLOR_PAIR(K_HUIS_MUUR);
            switch (c0) {
                case '#': c |= A_BOLD | A_DIM | COLOR_PAIR(K_MUUR); break;
                case '.': c = ACS_BULLET; break;
                case '+': c = ACS_HLINE | COLOR_PAIR(K_HUIS_DEUR); break;
                case '[':
                case ']': c = ACS_VLINE | huis_muur; break;
                case '-':
                case '_': c = ACS_HLINE | huis_muur; break;
                case '1': c = ACS_ULCORNER | huis_muur; break;
                case '2': c = ACS_URCORNER | huis_muur; break;
                case '3': c = ACS_LLCORNER | huis_muur; break;
                case '4': c = ACS_LRCORNER | huis_muur; break;
            }

            waddch(win, c);
        }
    }
}

/* Teken een spook
 *
 * win: het venster om op te tekenen
 * spook: het spook om te tekenen
 *
 * Side effects:
 * - tekst wordt naar het venster geschreven
 */
void teken_spook(WINDOW *win, const spook *spook) {
    mvwaddch(
        win,
        spook->ent.y + 1 + SCORE_HOOGTE,
        spook->ent.x + 1,
        '@' | COLOR_PAIR(spook->kleur)
    );
}

pacman *pm_maak(rooster *veld, int *hoogte, int *breedte) {
    pacman *data = malloc(sizeof(pacman));
    if (data == NULL) {
        perror("pm_maak");
        return NULL;
    }

    // 1. Vul veldgegevens in
    data->veld = veld;
    data->breedte = rooster_breedte(data->veld);
    data->hoogte = rooster_hoogte(data->veld);
    *breedte = data->breedte + 2;
    *hoogte = data->hoogte + 2 + SCORE_HOOGTE;

    // 2. Zet een aantal tellers op hun beginwaardes
    data->stappen = 0;
    data->levens = MAX_LEVENS;
    data->dood_animatie = -1;
    data->doel_rot = GEEN;

    // 3. Vul het veld met voedsel
    // (het vak waar de speler op begint telt als virtuele voedsel)
    data->begin_voedsel = 1;
    data->gegeten_voedsel = 1; 
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

    // 4. Maak de spoken
    data->blinky = maak_spook(&doel_blinky, K_BLINKY);
    data->pinky  = maak_spook(&doel_pinky,  K_PINKY);
    data->inky   = maak_spook(&doel_inky,   K_INKY);
    data->clyde  = maak_spook(&doel_clyde,  K_CLYDE);

    // 5. Vind alle beginplaatsen
    int ok = zoek_plaats(data->veld, '!', &data->start.speler, RECHTS) &&
             zoek_plaats(data->veld, 'B', &data->start.blinky, OMHOOG) &&
             zoek_plaats(data->veld, 'P', &data->start.pinky,  OMHOOG) &&
             zoek_plaats(data->veld, 'I', &data->start.inky,   OMHOOG) &&
             zoek_plaats(data->veld, 'C', &data->start.clyde,  OMHOOG);
    if (!ok) {
        fprintf(stderr, "een of meerde beginplaatsen ontbreken\n");
        pm_klaar(data);
        return NULL;
    }
    ga_naar_start(data);

    // 6. Zorg dat er een bruikbare deur is
    // (dit wordt als laatste gedaan, want eerdere stappen kunnen vrije ruimte maken)
    {
        int deur_x, deur_y;
        rooster_zoek(data->veld, '+', &deur_x, &deur_y);
        if (deur_x == -1) {
            fprintf(stderr, "er ontbreekt een deur ('+')\n");
            pm_klaar(data);
            return NULL;
        }

        if (!bewandelbaar(rooster_kijk(data->veld, deur_x, deur_y - 1))) {
            fprintf(stderr, "het vakje boven de deur moet bewandelbaar zijn\n");
            pm_klaar(data);
            return NULL;
        }
    }

    return data;
}

void pm_klaar(pacman *data) {
    if (data->veld) {
        rooster_klaar(data->veld);
    }
    free(data);
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

int pm_stap(pacman *data) {
    if (data->dood_animatie >= 0) {
        if (data->stappen % 5 == 0) {
            if (data->dood_animatie == 0) {
                ga_naar_start(data);
            }

            data->dood_animatie--;
        }

        data->stappen++;
        return 1;
    } else if (data->levens == 0) {
        return 0;
    }

    verander_speler_richting(data);

    if (data->stappen % 9 == 0) {
        stap_spook(data, &data->pinky);
        stap_spook(data, &data->inky);
        stap_spook(data, &data->clyde);
        stap_spook(data, &data->blinky);
    }

    stap_speler(data);

    data->stappen++;

    return 1;
}

void pm_teken(WINDOW *win, const pacman *data) {
    if (data->dood_animatie % 2 == 0) {
        wkleur_aan(win, K_HARTJE);
    }
    box(win, 0, 0);
    if (data->dood_animatie % 2 == 0) {
        wkleur_uit(win, K_HARTJE);
    }

    teken_score(win, data);
    teken_rooster(win, data->veld);

    const char *speler_chars = ((data->stappen / 5) % 2) ? SPELER_ALT : SPELER_STD;
    mvwaddch(
        win,
        data->speler.y + 1 + SCORE_HOOGTE,
        data->speler.x + 1,
        speler_chars[data->speler.rot] | COLOR_PAIR(K_PACMAN) | A_BOLD
    );

    teken_spook(win, &data->blinky);
    teken_spook(win, &data->pinky);
    teken_spook(win, &data->inky);
    teken_spook(win, &data->clyde);
}
