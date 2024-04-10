#include "raylib.h"
#include "screens.h"

void init_menu_game(void)
{
    // Init menu variables here
}


GameScreen draw_menu_screen(void) 
{
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();

    const char *sequence_text = "Press [S] to play the sequence";
    int sequence_text_size = 25;
    int sequence_text_width = MeasureText(sequence_text, sequence_text_size);

    const char *pairs_text = "Press [P] to play the pairs";
    int pairs_text_size = 25;
    int pairs_text_width = MeasureText(pairs_text, pairs_text_size);

    int pairs_text_x = screen_width/2 - pairs_text_width/2; 
    int pairs_text_y = screen_height/2 - (pairs_text_size + sequence_text_size + 10)/2; 
    DrawText(pairs_text, pairs_text_x, pairs_text_y, pairs_text_size, GREEN);

    int sequence_text_x = screen_width/2 - sequence_text_width/2;               
    int sequence_text_y = pairs_text_y + 10 + pairs_text_size; 
    DrawText(sequence_text, sequence_text_x, sequence_text_y, sequence_text_size, GREEN);
    
    if (IsKeyPressed(KEY_S)) {
        return SEQUENCE;
    } else if (IsKeyPressed(KEY_P)) {
        return PAIRS;
    } 

    return MENU;
}
