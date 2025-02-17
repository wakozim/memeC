#include "raylib.h"
#include "screens.h"

#define GUI_IMPLEMENTATION
#include "gui.h"

#ifdef PLATFORM_WEB
extern unsigned char __heap_base;

unsigned int bump_pointer = (int)(int*)&__heap_base;
void *_Nullable malloc(size_t n) {
    unsigned int r = bump_pointer;
    bump_pointer += n;
    return (void*)r;
}

void *_Nullable realloc(void *_Nullable ptr, size_t size) {
    return malloc(size);
}

void free(void *_Nullable p) {
  // lol
}
#endif // PLATFORM_WEB

static GameScreen current_screen = MENU;
static int mouse_cursor = MOUSE_CURSOR_DEFAULT;

static void update_draw_frame(void);
static void change_to_screen(int screen);
void raylib_js_set_entry(void (*entry)(void));


int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(800, 600, "memeC");

    change_to_screen(MENU);
    
#ifdef PLATFORM_WEB
    raylib_js_set_entry(update_draw_frame);
#else
    while (!WindowShouldClose()) {
        update_draw_frame();
    }

    CloseWindow();
#endif

    return 0;
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
}

