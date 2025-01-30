#include "raylib.h"
#include "screens.h"


GameScreen current_screen = MENU;
int mouse_cursor = MOUSE_CURSOR_DEFAULT;

static void update_draw_frame(void);


int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(800, 600, "memeC");

    while (!WindowShouldClose()) {
        update_draw_frame();
    }

    CloseWindow();
}

void set_mouse_cursor(int cursor)
{
    if (cursor != mouse_cursor) {
        SetMouseCursor(cursor);
        mouse_cursor = cursor;
    }
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

        GameScreen screen;

        switch(current_screen)
        {
            case MENU: {
                screen = draw_menu_screen();
            } break;
            case SEQUENCE: {
                screen = draw_sequence_screen();
            } break;
            case PAIRS: {
                screen = draw_pairs_screen();
            } break;
            default: break;
        }

        if (screen != current_screen) {
            change_to_screen(screen);
        }

    EndDrawing();
    //----------------------------------------------------------------------------------
}

