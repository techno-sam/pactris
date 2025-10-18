/* spel.h
 *
 * Deze module bevat types waarmee de sub spellen kunnen communiceren.
 *
 * Met de `toestand` enum geven Pacman en Tetris aan of het spel door moet gaan.
 *
 * Het callback systeem laat de spellen op elkaar reageren zonder elkaars logica te hoeven kennen.
 */

#ifndef _SPEL_H
#define _SPEL_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef enum {
    ST_PACMAN = 1,
    ST_TETRIS = 2,
    ST_ALLEBEI = ST_PACMAN | ST_TETRIS
} spel_type;

typedef enum {
    AAN_HET_SPELEN,
    GEWONNEN,
    VERLOREN,
    NEUTRAAL_KLAAR
} toestand;

typedef void (*int_callback_fn)(int arg, void *userdata);

typedef struct {
    int_callback_fn fn;
    void *userdata;
} int_callback;

/* Bereken het aantal cijfers in een getal
 *
 * v: het getal
 *
 * Uitvoer: het aantal cijfers
 */
int ceil_log10(int v);

#endif
