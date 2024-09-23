#include "raylib.h"
#include "screens.h"

#define BACKGROUND_COLOR ColorFromHSV(0, 0.00f, 0.10f)
#define TEXT_COLOR       GREEN

#define MENU_TEXT_SIZE 30


void init_menu_game(void)
{
    // Init menu variables here
}


GameScreen draw_menu_screen(void)
{
    ClearBackground(BACKGROUND_COLOR);

    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();

    const char *sequence_text = "Press [S] to play the sequence";
    int sequence_text_width = MeasureText(sequence_text, MENU_TEXT_SIZE);

    const char *pairs_text = "Press [P] to play the pairs";
    int pairs_text_width = MeasureText(pairs_text, MENU_TEXT_SIZE);

    int pairs_text_x = screen_width/2 - pairs_text_width/2;
    int pairs_text_y = screen_height/2 - (MENU_TEXT_SIZE * 2 + 10)/2;
    DrawText(pairs_text, pairs_text_x, pairs_text_y, MENU_TEXT_SIZE, TEXT_COLOR);

    int sequence_text_x = screen_width/2 - sequence_text_width/2;
    int sequence_text_y = pairs_text_y + 10 + MENU_TEXT_SIZE;
    DrawText(sequence_text, sequence_text_x, sequence_text_y, MENU_TEXT_SIZE, TEXT_COLOR);

    if (IsKeyPressed(KEY_S)) {
        return SEQUENCE;
    } else if (IsKeyPressed(KEY_P)) {
        return PAIRS;
    }

    return MENU;
}
