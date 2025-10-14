/* Voorbeeldje van het gebruik van ncurses.
   In dit voorbeeld kun je alleen met een + naar links en naar rechts wandelen
   over een lijn van -.
*/

#include <ncurses.h>

int RANGE = 20;

/* Toont de situatie met ncurses.

   pos: de positie van de +.
*/
void laat_zien(int pos) {
    clear();   // begin met een nieuw ncurses scherm
    for (int i = 0; i < RANGE; i++) {
        addch(i == pos ? '+' : '-');
    }
    refresh(); // zorg dat het scherm ook echt getoond wordt
}

int main(void) {
    // Initialiseer ncurses
    initscr();
    cbreak();             // zodat je kunt onderbreken met Ctrl+C
    keypad(stdscr, TRUE); // luister ook naar extra toetsen zoals pijltjes
    noecho();             // druk niet de letters af die je intypt

    int pos = RANGE / 2;    // begin in het midden van de lijn.
    while (1) {
        laat_zien(pos);
        int toets = getch();
        switch (toets) {
        case KEY_LEFT:
            pos--;
            break;
        case KEY_RIGHT:
            pos++;
            break;
        }
    }

    return 0;
}
