#include "raylib.h"
#include "screens.h"


GameScreen current_screen = MENU;

static void update_draw_frame(void);


int main(void) 
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(800, 600, "memeC");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        update_draw_frame();
    }

    CloseWindow();
}

static void change_to_screen(int screen) 
{
    switch (current_screen) {
        case MENU: break;
        case PAIRS: break;
        case SEQUENCE: break;
    }

    switch (screen) {
        case MENU: init_menu_game(); break;
        case PAIRS: init_pairs_game(); break;
        case SEQUENCE: init_sequence_game(); break;
    }


    current_screen = screen;
}


static void update_draw_frame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        switch(current_screen)
        {
            case MENU: { 
                    GameScreen screen = draw_menu_screen(); 
                    if (screen != MENU) {
                        change_to_screen(screen);
                    }
            } break;
            case SEQUENCE: draw_sequence_screen(); break;
            case PAIRS: draw_pairs_screen(); break;
            default: break;
        }

    EndDrawing();
    //----------------------------------------------------------------------------------
}

