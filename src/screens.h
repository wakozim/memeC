#ifndef SCREENS_H
#define SCREENS_H


typedef enum GameScreen {
    MENU,
    PAIRS,
    SEQUENCE
} GameScreen;

//--------------------------
// Menu
// -------------------------

void init_menu_game(void);
GameScreen draw_menu_screen(void);


//--------------------------
// Sequence
// -------------------------

void init_sequence_game(void);
void draw_sequence_screen(void);

//--------------------------
// Pairs
// -------------------------

int draw_pairs_screen(void);
void init_pairs_game(void);

#endif // SCREENS_H
