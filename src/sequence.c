#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

#define SEQUENCE_CAP 3
#define BOARD_LINES 3
#define BOARD_COLUMNS 3
#define BOARD_CAP (BOARD_LINES * BOARD_COLUMNS)

#define BAR_CIRCLE_RADIUS 10
#define BAR_CIRCLE_GAP 10
#define BAR_HEIGHT (BAR_CIRCLE_RADIUS*2 + BAR_CIRCLE_GAP*2)
#define BAR_WIDTH ((BAR_CIRCLE_RADIUS*2 * SEQUENCE_CAP) + (BAR_CIRCLE_GAP * (SEQUENCE_CAP - 1)))
#define CELL_SIZE 100
#define CELL_CLICKED_SIZE (CELL_SIZE - 10)
#define CELL_GAP_SIZE 10
#define BOARD_WIDTH ((CELL_SIZE * BOARD_COLUMNS) + (CELL_GAP_SIZE * (BOARD_COLUMNS - 1)))
#define BOARD_HEIGHT ((CELL_SIZE * BOARD_LINES) + (CELL_GAP_SIZE * (BOARD_LINES - 1)))

#define CELL_DEFAULT_COLOR        ColorFromHSV(0, 0.00f, 0.30f)
#define CELL_SHOWCASE_COLOR       ColorFromHSV(0, 0.55f, 0.85f)
#define CELL_USER_CHOICE_COLOR    ColorFromHSV(120, 0.55f, 0.85f)
#define BAR_CIRCLE_ACTIVE_COLOR   ColorFromHSV(0, 0.00f, 0.80f)
#define BAR_CIRCLE_INACTIVE_COLOR ColorFromHSV(0, 0.00f, 0.30f)
#define BACKGROUND_COLOR          ColorFromHSV(0, 0.00f, 0.10f)


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
    int index;
    int sequence[SEQUENCE_CAP];
    int sequence_lenght;
    int user_guess_lenght;
    int user_guess_index;
} Game;

Game game = {0};

bool add_number_to_sequence(void)
{
    int old_lenght = game.sequence_lenght;
    game.sequence_lenght += 1;

    if (old_lenght >= SEQUENCE_CAP)
        return true;

    game.sequence[old_lenght] = rand() % BOARD_CAP;
    return false;
}

void restart_game(void)
{
    game.state = SHOW_SEQUENCE;
    game.time = 0.0f;
    game.sequence_lenght = 0;
    game.user_guess_lenght = 0;
    game.user_guess_index = -1;

    for (int i = 0; i < SEQUENCE_CAP; i++)
        game.sequence[i] = -1;

    add_number_to_sequence();
}

void init_game(void)
{
    restart_game();
}


void print_sequence(void)
{
    for (int i = 0; i < SEQUENCE_CAP; i++)
        printf("sequence[%d] = %d\n", i, game.sequence[i]);
}

void init_sequence_game(void)
{
    srand(time(NULL));
    init_game();
    game.state = SHOW_SEQUENCE;
    game.time = 0.0f;
    game.index = 0;
}

void draw_bar(void)
{
    int bar_cx = GetScreenWidth() / 2 - (BAR_WIDTH / 2);
    int bar_cy = GetScreenHeight() / 2 - (BAR_HEIGHT + BOARD_HEIGHT / 2);
    bar_cx += BAR_CIRCLE_RADIUS;
    bar_cy += BAR_CIRCLE_RADIUS;
    for (int i = 0; i < SEQUENCE_CAP; i++) {
        int x_gap = BAR_CIRCLE_GAP * i;
        Color color = game.sequence_lenght > i + 1 ? BAR_CIRCLE_ACTIVE_COLOR : BAR_CIRCLE_INACTIVE_COLOR;
        DrawCircle(bar_cx + i * (BAR_CIRCLE_RADIUS*2) + x_gap, bar_cy, BAR_CIRCLE_RADIUS, color);
    }
}


void draw_field(void)
{
    int board_cx = (GetScreenWidth() / 2) - (BOARD_WIDTH / 2);
    int board_cy = (GetScreenHeight() / 2) - (BAR_HEIGHT + BOARD_HEIGHT / 2);
    board_cy += BAR_HEIGHT;
    for (int iy = 0; iy < BOARD_LINES; iy++) {
        int y_gap = CELL_GAP_SIZE * iy;
        for (int ix = 0; ix < BOARD_COLUMNS; ix++) {
            int x_gap = CELL_GAP_SIZE * ix;
            int cell_index = (iy * BOARD_LINES + ix);
            Rectangle cell_rect = { board_cx + ix*CELL_SIZE + x_gap, board_cy + iy*CELL_SIZE + y_gap, CELL_SIZE, CELL_SIZE};
            bool is_hovered = CheckCollisionPointRec(GetMousePosition(), cell_rect);

            Color color = CELL_DEFAULT_COLOR;
            switch (game.state) {
            case SHOW_SEQUENCE: {
                if (game.time > 0.2f && game.sequence[game.index] == cell_index)
                    color = CELL_SHOWCASE_COLOR;
            } break;
            case USER_GUESS:
            case USER_GUESS_WRONG:
            case USER_GUESS_CORRECT: {
                if (game.user_guess_index == cell_index)
                    color = CELL_USER_CHOICE_COLOR;
            } break;
            default:
                break;
            }

            DrawRectangleRec(cell_rect, color);

            if (game.state == USER_GUESS) {
                if (is_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    game.user_guess_index = cell_index;
                    if (game.sequence[game.user_guess_lenght] == cell_index) {
                        game.user_guess_lenght += 1;
                        if (game.user_guess_lenght >= game.sequence_lenght) {
                            game.state = USER_GUESS_CORRECT;
                            game.time = 0.0f;
                        }
                    } else {
                        game.time = 0.0f;
                        game.state = USER_GUESS_WRONG;
                    }
                }
            }
        }
    }
}

void draw_sequence_screen(void)
{
    ClearBackground(BACKGROUND_COLOR);

    if (IsKeyPressed(KEY_R)) {
        restart_game();
    }

    if (game.time >= 1.0f) {
        switch (game.state) {
        case SHOW_SEQUENCE: {
            game.index += 1;
            if (game.index >= game.sequence_lenght) {
                game.index = 0;
                game.state = USER_GUESS;
            }
            game.time = 0.0f;
        } break;
        case USER_GUESS_WRONG: {
            game.state = SHOW_SEQUENCE;
            game.user_guess_lenght = 0;
            game.user_guess_index = -1;
            game.time = 0.0f;
        } break;
        case USER_GUESS_CORRECT: {
            game.user_guess_lenght = 0;
            game.user_guess_index = -1;
            if (add_number_to_sequence())
                game.state = USER_WIN;
            else
                game.state = SHOW_SEQUENCE;
            game.time = 0.0f;
        } break;
        default:
            break;
        }
    } else {
        game.time += GetFrameTime();
    }

    draw_bar();
    draw_field();

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


// TODO: When the same cell appears in sequence twice in a row, it is not very obvious
// TODO: Maybe make particles?
// TODO: Sounds?
