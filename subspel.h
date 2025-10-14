/* TODO: HEADER COMMENT */

#ifndef _SUBSPEL_H
#define _SUBSPEL_H

#include <ncurses.h>

typedef enum {
    TEKEN_VERS, // bijvoorbeeld als het scherm groter/kleiner is geworden
    TEKEN_INCREMENTEEL
} teken_mode;

// (win, data) -> ()
typedef void (*subs_init)(WINDOW *, void *);
// (win, toets, data) -> gebruikt: bool
typedef int (*subs_toets)(WINDOW *, int, void *);
// (win, data) -> ()
typedef void (*subs_stap)(WINDOW *, void *);
// (win, teken_mode, data) -> ()
typedef void (*subs_teken)(WINDOW *, teken_mode, void *);
// (data) -> ()
typedef void (*subs_klaar)(void *);

typedef struct {
    subs_init init;
    subs_toets toets;
    subs_stap stap;
    subs_teken teken;
    subs_klaar klaar;
    void *data;
} subspel;

#endif
