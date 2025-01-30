#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"

#include "screens.h"


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
#define CELL_HOVERED_COLOR        ColorFromHSV(0, 0.00f, 0.40f)
#define BAR_CIRCLE_ACTIVE_COLOR   ColorFromHSV(0, 0.00f, 0.80f)
#define BAR_CIRCLE_INACTIVE_COLOR ColorFromHSV(0, 0.00f, 0.30f)
#define BACKGROUND_COLOR          ColorFromHSV(0, 0.00f, 0.10f)

#define MAX_SHOW_SEQUENCE_TIME 1.0f
#define MAX_CELL_PRESSING_TIME 0.25f
#define MAX_USER_GUESSED_TIME  1.25f


typedef enum {
    STATE_SHOW_SEQUENCE,
    STATE_USER_GUESS,
    STATE_USER_PRESS_CELL,
    STATE_USER_GUESS_WRONG,
    STATE_USER_GUESS_CORRECT,
    STATE_USER_WIN,
    STATE_USER_LOSE
} State;

typedef enum CellState {
    CELL_NONE = 0,
    CELL_INCREASE,
    CELL_DECREASE,
} CellState;

typedef struct Cell {
    float time;
    CellState state;
} Cell;

typedef struct Game {
    State state;
    float time;
    Cell board[BOARD_CAP];
    int sequence[SEQUENCE_CAP];
    int sequence_show_index;
    int sequence_len;
    int user_guess_len;
    int pressed_cell;
    int mouse_cursor;
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
    game.state = STATE_SHOW_SEQUENCE;
    game.time = MAX_SHOW_SEQUENCE_TIME;
    game.sequence_show_index = 0;
    game.sequence_len = 0;
    game.user_guess_len = 0;
    game.pressed_cell = -1;

    for (int i = 0; i < SEQUENCE_CAP; ++i) {
        game.sequence[i] = -1;
    }

    for (int i = 0; i < BOARD_CAP; ++i) {
        game.board[i].time = 0.0f;
        game.board[i].state = CELL_NONE;
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
            case STATE_USER_GUESS_CORRECT: {
                if (game.sequence_len != i + 1) break;
                float t = 1.0f - game.time / MAX_USER_GUESSED_TIME;
                color = ColorLerp(color, BAR_CIRCLE_ACTIVE_COLOR, t);
                radius += Lerp(0, 3, sinf(t*PI));
            } break;
            case STATE_USER_GUESS_WRONG: {
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
    if (game.state == STATE_USER_PRESS_CELL && game.board[i].state == CELL_DECREASE) {
        if (game.board[i].time <= 0.0f) {
            game.board[i].time = MAX_CELL_PRESSING_TIME;
            game.board[i].state = CELL_INCREASE;
        }
    }

    if (game.board[i].state == CELL_INCREASE && game.board[i].time <= 0.0f) {
        game.board[i].time = 0.0f;
        game.board[i].state = CELL_NONE;
    }

    if (game.state != STATE_USER_GUESS) return;

    if (is_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game.pressed_cell = i;
        game.board[i].time = MAX_CELL_PRESSING_TIME;
        game.board[i].state = CELL_DECREASE;
        return;
    }

    int pressed_cell = game.pressed_cell;
    if (pressed_cell != i) return;

    if (!is_hovered) {
        game.pressed_cell = -1;
        game.board[i].time = MAX_CELL_PRESSING_TIME;
        game.board[i].state = CELL_INCREASE;
        return;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        game.state = STATE_USER_PRESS_CELL;
        return;
    }
}


static void draw_board(void)
{
    int board_x = (GetScreenWidth()/2) - (FIELD_WIDTH/2);
    int board_y = (GetScreenHeight()/2) - (FIELD_HEIGHT/2) + BAR_HEIGHT;

    for (int i = 0; i < BOARD_CAP; ++i) {
        if (game.board[i].time >= 0.0f) game.board[i].time -= GetFrameTime();
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
        bool is_hovered = CheckCollisionPointRec(
            GetMousePosition(),
            cell_rect
        );

        switch (game.state) {
            case STATE_SHOW_SEQUENCE: {
                if (game.sequence[game.sequence_show_index] != i) break;
                float t = 1.0f - game.time / MAX_SHOW_SEQUENCE_TIME;
                color = ColorLerp(color, CELL_SHOWCASE_COLOR, 2*sinf((t*PI)));
                offset = Lerp(0.0f, -3.0f, 2.0f*sinf(t*PI));
            } break;
            case STATE_USER_PRESS_CELL: {
                float t = game.board[i].time / MAX_CELL_PRESSING_TIME;
                if (game.board[i].state == CELL_INCREASE) {
                    offset = Lerp(0.0f, -1.0f, 2*PI*sinf(t));
                } else if (game.board[i].state == CELL_DECREASE) {
                    offset = Lerp(0.0f, -1.0f, 2*PI*sinf(1.0f - t));
                }
                if (game.pressed_cell != i) break;
                color = CELL_SHOWCASE_COLOR;
            } break;
            case STATE_USER_GUESS: {
                float t = 0.0f;
                if (game.board[i].state == CELL_INCREASE) {
                    t = game.board[i].time / MAX_CELL_PRESSING_TIME;
                } else if (game.board[i].state == CELL_DECREASE) {
                    t = 1.0f - game.board[i].time / MAX_CELL_PRESSING_TIME;
                }

                offset = Lerp(0.0f, -1.0f, 2*PI*sinf(t));
                color = is_hovered ? CELL_HOVERED_COLOR : color;
                if (is_hovered) {
                    game.mouse_cursor = MOUSE_CURSOR_POINTING_HAND;
                }
            } break;
            case STATE_USER_GUESS_CORRECT: {
                float t = 1.0f - game.time / MAX_USER_GUESSED_TIME;
                color = ColorLerp(color, CELL_SHOWCASE_COLOR, sinf(t*2*PI));
            } break;
            case STATE_USER_GUESS_WRONG: {
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

GameScreen draw_sequence_screen(void)
{
    GameScreen result = SEQUENCE;
    game.mouse_cursor = MOUSE_CURSOR_DEFAULT;

    ClearBackground(BACKGROUND_COLOR);

    if (game.time >= 0.0f) game.time -= GetFrameTime();

    if (IsKeyPressed(KEY_R)) {
        restart_game();
    } else if (IsKeyPressed(KEY_Q)) {
        result = MENU;
    }

    switch (game.state) {
        case STATE_SHOW_SEQUENCE: {
            if (game.time >= 0.0f) break;
            game.time = MAX_SHOW_SEQUENCE_TIME;
            ++game.sequence_show_index;
            if (game.sequence_show_index >= game.sequence_len) {
                game.state = STATE_USER_GUESS;
                game.sequence_show_index = 0;
                game.time = 0.0f;
            }
        } break;
        case STATE_USER_GUESS_CORRECT: {
            if (game.time >= 0.0f) break;
            game.user_guess_len = 0;
            game.state = STATE_SHOW_SEQUENCE;
            game.time = MAX_SHOW_SEQUENCE_TIME;
            if (add_number_to_sequence()) {
                game.state = STATE_USER_WIN;
            }
        } break;
        case STATE_USER_GUESS_WRONG: {
            if (game.time >= 0.0f) break;
            game.user_guess_len = 0;
            game.state = STATE_SHOW_SEQUENCE;
            game.time = MAX_SHOW_SEQUENCE_TIME;
        } break;
        case STATE_USER_PRESS_CELL: {
            if (game.board[game.pressed_cell].state != CELL_NONE) break;

            int pressed_cell = game.pressed_cell;
            game.pressed_cell = -1;
            if (game.sequence[game.user_guess_len] != pressed_cell) {
                game.state = STATE_USER_GUESS_WRONG;
                game.time = MAX_USER_GUESSED_TIME;
                game.user_guess_len = 0;
                break;
            }

            ++game.user_guess_len;
            if (game.user_guess_len >= game.sequence_len) {
                game.state = STATE_USER_GUESS_CORRECT;
                game.time = MAX_USER_GUESSED_TIME;
                game.user_guess_len = 0;
                break;
            }
            game.state = STATE_USER_GUESS;
        } break;
        default: break;
    }

    draw_bar();
    draw_board();

    if (game.state == STATE_USER_WIN) {
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

    set_mouse_cursor(game.mouse_cursor);

    return result;
}


// TODO: Maybe make particles?
// TODO: Sounds?
