#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rooster.h"

struct rooster_data {
    int breedte;
    int hoogte;
    char *data;
};

rooster *rooster_lees(FILE *fh) {
    if (fh == NULL) {
        return NULL;
    }

    rooster *r = malloc(sizeof(rooster));
    if (r == NULL) {
        perror("rooster_lees: r");
        return NULL;
    }
    r->breedte = 0;
    r->hoogte = 0;

    size_t totale_grootte = 0;
    size_t buf_lengte = 64;
    r->data = malloc(buf_lengte * sizeof(char));
    if (r->data == NULL) {
        perror("rooster_lees: r->data");
        return NULL;
    }

    while (1) {
        totale_grootte += fread(r->data + totale_grootte, 1, buf_lengte - totale_grootte, fh);

        // buffer vol, moet groter
        if (totale_grootte == buf_lengte) {
            buf_lengte *= 2;
            r->data = realloc(r->data, buf_lengte);
            if (r->data == NULL) {
                perror("rooster_lees: realloc r->data");
                free(r);
                return NULL;
            }
        }

        // helemaal ingelezen
        if (feof(fh)) {
            r->data[totale_grootte] = '\0';
            break;
        }

        // misschien ontplofte de harde schijf?
        if (ferror(fh)) {
            fprintf(stderr, "rooster_lees: lees error\n");
            free(r->data);
            free(r);
            return NULL;
        }
    }

    // vind de breedte en dan de hoogte
    int regel_breedte = 0;
    for (int i = 0; r->data[i] != '\0'; i++) {
        if (r->data[i] == '\n') {
            if (r->hoogte == 0) { // meet de breedte van de eerste regel
                if (regel_breedte == 0) {
                    fprintf(stderr, "rooster_lees: een doolhof mag niet"
                                    " een breedte van nul hebben\n");
                    free(r->data);
                    free(r);
                    return NULL;
                }

                r->breedte = regel_breedte;
            } else if (r->breedte != regel_breedte) {
                fprintf(stderr, "rooster_lees: alle regels moeten even lang zijn\n");
                free(r->data);
                free(r);
                return NULL;
            }

            regel_breedte = 0;
            r->hoogte++;
        } else {
            regel_breedte++;
        }
    }

    if (regel_breedte != 0) {
        fprintf(stderr, "rooster_lees: een doolhof moet eindigen op een newline\n");
        free(r->data);
        free(r);
        return NULL;
    }

    if (r->hoogte == 0 || r->breedte == 0) {
        fprintf(stderr, "rooster_lees: een doolhof moet minstens een rij en een kolom hebben\n");
        free(r->data);
        free(r);
        return NULL;
    }

    return r;
}

void rooster_klaar(rooster *rp) {
    free(rp->data);
    free(rp);
}

int rooster_breedte(const rooster *rp) {
    return rp->breedte;
}

int rooster_hoogte(const rooster *rp) {
    return rp->hoogte;
}

int rooster_bevat(const rooster *rp, int x, int y) {
    return 0 <= x && x < rp->breedte &&
           0 <= y && y < rp->hoogte;
}

char rooster_kijk(const rooster *rp, int x, int y) {
    if (rooster_bevat(rp, x, y)) {
        return rp->data[x + (rp->breedte + 1) * y];
    } else {
        return '\0';
    }
}

int rooster_plaats(rooster *rp, int x, int y, char c) {
    if (rooster_bevat(rp, x, y)) {
        rp->data[x + (rp->breedte + 1) * y] = c;
        return 1;
    } else {
        return 0;
    }
}

void rooster_zoek(const rooster *rp, char c, int *x, int *y) {
    for (int j = 0; j < rp->hoogte; j++) {
        for (int i = 0; i < rp->breedte; i++) {
            if (rp->data[i + (rp->breedte + 1) * j] == c) {
                *x = i;
                *y = j;
                return;
            }
        }
    }

    *x = -1;
    *y = -1;
}
