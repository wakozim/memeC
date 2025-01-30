#ifndef SCREENS_H
#define SCREENS_H


typedef enum GameScreen {
    MENU,
    PAIRS,
    SEQUENCE
} GameScreen;

void set_mouse_cursor(int cursor);

//--------------------------
// Menu
// -------------------------

void init_menu_game(void);
GameScreen draw_menu_screen(void);


//--------------------------
// Sequence
// -------------------------

void init_sequence_game(void);
GameScreen draw_sequence_screen(void);

//--------------------------
// Pairs
// -------------------------

void init_pairs_game(void);
GameScreen draw_pairs_screen(void);

#endif // SCREENS_H
