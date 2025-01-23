#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"

#define SEQUENCE_CAP 5
#define BOARD_LINES 3
#define BOARD_COLUMNS 3
#define BOARD_CAP ((BOARD_LINES) * (BOARD_COLUMNS))

#define BAR_CIRCLE_RADIUS 10
#define BAR_CIRCLE_GAP    10
#define CELL_SIZE         100
#define CELL_GAP          10
#define CELL_PRESSED_SIZE ((CELL_SIZE) - 10)
#define BAR_HEIGHT        (((BAR_CIRCLE_RADIUS)*2) + ((BAR_CIRCLE_GAP)*2))
#define BAR_WIDTH         ((((BAR_CIRCLE_RADIUS)*2) * (SEQUENCE_CAP)) + ((BAR_CIRCLE_GAP) * ((SEQUENCE_CAP) - 1)))
#define BOARD_WIDTH       ((CELL_SIZE * BOARD_COLUMNS) + (CELL_GAP * (BOARD_COLUMNS - 1)))
#define BOARD_HEIGHT      ((CELL_SIZE * BOARD_LINES) + (CELL_GAP * (BOARD_LINES - 1)))
#define FIELD_HEIGHT      ((BOARD_HEIGHT) + (BAR_HEIGHT))
#if BAR_WIDTH < BOARD_WIDTH
#   define FIELD_WIDTH    (BOARD_WIDTH)
#else
#   define FILED_WIDTH    (BAR_WIDTH)
#endif

#define CELL_DEFAULT_COLOR        ColorFromHSV(0, 0.00f, 0.30f)
#define CELL_SHOWCASE_COLOR       ColorFromHSV(0, 0.55f, 0.85f)
#define CELL_HOVERED_COLOR        ColorFromHSV(120, 0.55f, 0.85f)
#define CELL_PRESSED_COLOR        ColorFromHSV(120, 0.55f, 0.65f)
#define BAR_CIRCLE_ACTIVE_COLOR   ColorFromHSV(0, 0.00f, 0.80f)
#define BAR_CIRCLE_INACTIVE_COLOR ColorFromHSV(0, 0.00f, 0.30f)
#define BACKGROUND_COLOR          ColorFromHSV(0, 0.00f, 0.10f)

#define MAX_SHOW_SEQUENCE_TIME 1.0f
#define MAX_CELL_PRESSING_TIME 0.25f
#define MAX_USER_GUESSED_TIME  1.25f


typedef enum {
    NONE = 0,
    SHOW_SEQUENCE,
    USER_GUESS,
    USER_GUESS_WRONG,
    USER_GUESS_CORRECT,
    USER_WIN,
    USER_LOSE
} State;

typedef struct Game {
    State state;
    float time;
    int sequence[SEQUENCE_CAP];
    int sequence_show_index;
    int sequence_len;
    int user_guess_len;
    int pressed_cell;
} Game;

Game game = {0};

void print_sequence(void)
{
    for (int i = 0; i < SEQUENCE_CAP; i++)
        printf("sequence[%d] = %d\n", i, game.sequence[i]);
}


bool add_number_to_sequence(void)
{
    int old_len = game.sequence_len;
    game.sequence_len += 1;

    if (old_len >= SEQUENCE_CAP)
        return true;

    game.sequence[old_len] = rand() % BOARD_CAP;
    return false;
}


void restart_game(void)
{
    game.state = SHOW_SEQUENCE;
    game.time = MAX_SHOW_SEQUENCE_TIME;
    game.sequence_show_index = 0;
    game.sequence_len = 0;
    game.user_guess_len = 0;

    for (int i = 0; i < SEQUENCE_CAP; i++) {
        game.sequence[i] = -1;
    }

    add_number_to_sequence();
}

void init_game(void)
{
    restart_game();
}


void init_sequence_game(void)
{
    srand(time(NULL));
    init_game();
}

void draw_bar(void)
{
    int bar_x = (GetScreenWidth()/2) - (BAR_WIDTH/2);
    int bar_y = (GetScreenHeight()/2) - (FIELD_HEIGHT/2);
    bar_x += BAR_CIRCLE_RADIUS;
    bar_y += BAR_CIRCLE_RADIUS;
    for (int i = 0; i < SEQUENCE_CAP; i++) {
        int x_gap = BAR_CIRCLE_GAP * i;
        float radius = BAR_CIRCLE_RADIUS; 
        float x = bar_x + i * (BAR_CIRCLE_RADIUS*2) + x_gap;
        float y = bar_y;
        Color color = game.sequence_len > i + 1 ? BAR_CIRCLE_ACTIVE_COLOR : BAR_CIRCLE_INACTIVE_COLOR;
        switch (game.state) {
            case USER_GUESS_CORRECT: {
                if (game.sequence_len != i + 1) break;
                float t = 1.0f - game.time / MAX_USER_GUESSED_TIME;
                color = ColorLerp(color, BAR_CIRCLE_ACTIVE_COLOR, t); 
                radius += Lerp(0, 3, sinf(t*PI));
            } break;
            case USER_GUESS_WRONG: {
                if (game.sequence_len != i + 1) break;
                float t = 1.0f - game.time / MAX_USER_GUESSED_TIME;
                color = ColorLerp(color, RED, t); 
                y += Lerp(0, 3, sinf(8*t*PI));
            } break;
            default: break;
        }
        DrawCircle(x, y, radius, color);
    }
}


static void cell_event_handler(int i, bool is_hovered)
{
    if (!is_hovered) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game.pressed_cell = i;
        game.time = MAX_CELL_PRESSING_TIME;
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (game.pressed_cell != i) return;
        game.pressed_cell = -1;

        if (game.sequence[game.user_guess_len] != i) {
            game.state = USER_GUESS_WRONG;
            game.time = MAX_USER_GUESSED_TIME;
            return;
        }
        
        ++game.user_guess_len;
        if (game.user_guess_len >= game.sequence_len) {
            game.state = USER_GUESS_CORRECT;
            game.time = MAX_USER_GUESSED_TIME;
        }
    }
}


static void draw_board(void)
{
    int board_x = (GetScreenWidth()/2) - (FIELD_WIDTH/2);
    int board_y = (GetScreenHeight()/2) - (FIELD_HEIGHT/2) + BAR_HEIGHT;

    for (int i = 0; i < BOARD_CAP; ++i) {
        int row = i / BOARD_COLUMNS;
        int col = i % BOARD_COLUMNS;
        
        
        Rectangle cell_rect = {
            .x = board_x + (col*CELL_SIZE) + (col*CELL_GAP),
            .y = board_y + (row*CELL_SIZE) + (row*CELL_GAP), 
            .width = CELL_SIZE, 
            .height = CELL_SIZE 
        };

        Color color = CELL_DEFAULT_COLOR;
        float offset = 0.0f;
        bool is_hovered = false; 

        switch (game.state) {
            case SHOW_SEQUENCE: {
                if (game.sequence[game.sequence_show_index] != i) break;
                float t = 1.0f - game.time / MAX_SHOW_SEQUENCE_TIME;
                color = ColorLerp(color, CELL_SHOWCASE_COLOR, 2*sinf((t*PI)));
                offset = -1.0f * Lerp(0, 3, 2*sinf(t*PI)); 
            } break;
            case USER_GUESS: {
                is_hovered = CheckCollisionPointRec(
                    GetMousePosition(),
                    cell_rect
                );

                if (!is_hovered) { 
                    game.pressed_cell = game.pressed_cell == i ? -1 : game.pressed_cell;
                    break;
                }
                color = CELL_HOVERED_COLOR;

                if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) || game.pressed_cell != i) break;
                float t = 1.0f - game.time / MAX_CELL_PRESSING_TIME;
                color = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? CELL_PRESSED_COLOR : color;
                offset = -1.0f * Lerp(0, 1, 2*PI*sinf(t)); 
            } break;
            case USER_GUESS_CORRECT: {
                float t = 1.0f - game.time / MAX_USER_GUESSED_TIME;
                color = ColorLerp(color, CELL_SHOWCASE_COLOR, sinf(t*2*PI));
            } break;
            case USER_GUESS_WRONG: {
                float t = 1.0f - game.time / MAX_USER_GUESSED_TIME;
                cell_rect.x -= Lerp(0, 5, sinf(t*8*PI));
            } break;
            default: break;
        }
        
        cell_rect.width = cell_rect.width + offset*2;
        cell_rect.x  -= offset;
        cell_rect.height = cell_rect.height + offset*2;
        cell_rect.y  -= offset;

        DrawRectangleRec(cell_rect, color);
        
        cell_event_handler(i, is_hovered); 
    }
}

void draw_sequence_screen(void)
{
    ClearBackground(BACKGROUND_COLOR);
    
    if (game.time >= 0.0f) game.time -= GetFrameTime();

    if (IsKeyPressed(KEY_R)) {
        restart_game();
    }
    
    switch (game.state) {
        case SHOW_SEQUENCE: {
            if (game.time >= 0.0f) break;
            game.time = MAX_SHOW_SEQUENCE_TIME;
            ++game.sequence_show_index;
            if (game.sequence_show_index >= game.sequence_len) {
                game.state = USER_GUESS;
                game.sequence_show_index = 0;
                game.time = 0.0f;
            }
        } break;
        case USER_GUESS_CORRECT: {
            if (game.time >= 0.0f) break;
            game.user_guess_len = 0;
            game.state = SHOW_SEQUENCE;
            game.time = MAX_SHOW_SEQUENCE_TIME;
            if (add_number_to_sequence()) {
                game.state = USER_WIN;
            }
        } break;
        case USER_GUESS_WRONG: {
            if (game.time >= 0.0f) break;
            game.user_guess_len = 0;
            game.state = SHOW_SEQUENCE;
            game.time = MAX_SHOW_SEQUENCE_TIME;
        } break;
        default: break;
    }

    draw_bar();
    draw_board();

    if (game.state == USER_WIN) {
        int height = 200;
        int width = 400;
        int x = GetScreenWidth() / 2 - width / 2;
        int y = GetScreenHeight() / 2 - height / 2;
        Color color = { .r = 0x69, .g = 0x69, .b = 0x69, .a = 0xEF};

        DrawRectangle(x, y, width, height, color);

        const char *restart_text = "Press [R] for restart";
        int restart_text_size = 25;
        int restart_text_width = MeasureText(restart_text, restart_text_size);

        const char *win_text = "You win!";
        int win_text_size = 69;
        int win_text_width = MeasureText(win_text, win_text_size);

        int win_text_x = x + width/2 - win_text_width/2;
        int win_text_y = y + height/2 - (win_text_size + restart_text_size + 10)/2;
        DrawText(win_text, win_text_x, win_text_y, win_text_size, GREEN);

        int restart_text_x = x + width/2 - restart_text_width/2;
        int restart_text_y = win_text_y + 10 + win_text_size;
        DrawText(restart_text, restart_text_x, restart_text_y, restart_text_size, GREEN);
    }
}


// TODO: Maybe make particles?
// TODO: Sounds?
