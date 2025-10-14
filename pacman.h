/* TODO: HEADER COMMENT */

#ifndef _PACMAN_H
#define _PACMAN_H

#include <ncurses.h>
#include <stdio.h>

#include "rooster.h"

struct pacman_data;
typedef struct pacman_data pacman;

pacman *pm_maak(rooster *veld, int *hoogte, int *breedte);

int pm_toets(int toets, pacman *data);

int pm_stap(pacman *data);

void pm_teken(WINDOW *win, pacman *data);

void pm_klaar(pacman *data);

#endif
