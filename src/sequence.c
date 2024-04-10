#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"


#define SEQUENCE_CAP 5
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

#define CELL_DEFAULT_COLOR WHITE 
#define CELL_SHOWCASE_COLOR RED 
#define CELL_USER_CHOICE_COLOR GREEN 
#define BAR_CIRCLE_ACTIVE_COLOR WHITE
#define BAR_CIRCLE_INACTIVE_COLOR GRAY 
#define BACKGROUND_COLOR CLITERAL(Color) {18, 18, 18, 0xFF}


static int sequence[SEQUENCE_CAP] = {1, 1, 1};
static int sequence_lenght = 3;

static int user_guess_lenght = 0;
static int user_guess_index = -1;


typedef enum {
    NONE = 0,
    SHOW_SEQUENCE,
    USER_GUESS, 
    USER_GUESS_WRONG,
    USER_GUESS_CORRECT,
    USER_WIN,
    USER_LOSE
} GameState;

GameState state;

static float ttime = 0.0f;
static int index = 0;


bool add_number_to_sequence(void) 
{
    int old_lenght = sequence_lenght;
    sequence_lenght += 1;

    if (old_lenght >= SEQUENCE_CAP) 
        return true;
    
    sequence[old_lenght] = rand() % BOARD_CAP;
    return false;
}


void init_game(void) 
{
    sequence_lenght = 0;
    user_guess_lenght = 0;
    user_guess_index = -1;

    for (int i = 0; i < SEQUENCE_CAP; i++) 
        sequence[i] = -1;

    add_number_to_sequence(); 
}


void print_sequence(void) 
{
    for (int i = 0; i < SEQUENCE_CAP; i++)
        printf("sequence[%d] = %d\n", i, sequence[i]); 
}

void init_sequence_game(void)
{
    srand(time(NULL));
    init_game();
    //state = SHOW_SEQUENCE;
    state = USER_WIN;
    ttime = 0.0f;
    index = 0;
}

void draw_sequence_screen(void) 
{
            ClearBackground(BACKGROUND_COLOR);

            if (IsKeyPressed(KEY_R)) {
                init_game();
                state = SHOW_SEQUENCE;
                ttime = 0.0f;
            }
#if 0 
            int ty = 0;
            DrawText(TextFormat("State = %d", state), 0, ty, 30, RED); 
            ty += 30;
            DrawText(TextFormat("sequence_lenght = %d", sequence_lenght), 0, ty, 30, RED); 
            ty += 30;
            DrawText(TextFormat("user_guess_index = %d", user_guess_index), 0, ty, 30, RED); 
            ty += 30;
            DrawText(TextFormat("index = %d", index), 0, ty, 30, RED); 
            ty += 30;
            DrawText(TextFormat("time = %f", time), 0, ty, 30, RED); 
#endif //DEBUG

            if (ttime >= 1.0f) {
                switch (state) {
                case SHOW_SEQUENCE: {
                    index += 1;
                    if (index >= sequence_lenght) {
                        index = 0;
                        state = USER_GUESS;
                    }               
                    ttime = 0.0f;
                } break;
                case USER_GUESS_WRONG: {
                    state = SHOW_SEQUENCE;
                    user_guess_lenght = 0;
                    user_guess_index = -1;
                    ttime = 0.0f;                 
                } break; 
                case USER_GUESS_CORRECT: {
                    user_guess_lenght = 0;
                    user_guess_index = -1;
                    if (add_number_to_sequence()) 
                        state = USER_WIN; 
                    else 
                        state = SHOW_SEQUENCE;
                    ttime = 0.0f;                   
                } break;
                default: 
                    break;
                } 
            } else {
                ttime += GetFrameTime();
            } 

            int bar_cx = GetScreenWidth() / 2 - (BAR_WIDTH / 2);
            int bar_cy = GetScreenHeight() / 2 - (BAR_HEIGHT + BOARD_HEIGHT / 2);
            bar_cx += BAR_CIRCLE_RADIUS;
            bar_cy += BAR_CIRCLE_RADIUS;
            for (int i = 0; i < SEQUENCE_CAP; i++) {
                int x_gap = BAR_CIRCLE_GAP * i; 
                Color color = sequence_lenght > i + 1 ? BAR_CIRCLE_ACTIVE_COLOR : BAR_CIRCLE_INACTIVE_COLOR;
                DrawCircle(bar_cx + i * (BAR_CIRCLE_RADIUS*2) + x_gap, bar_cy, BAR_CIRCLE_RADIUS, color);
            }

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
                    switch (state) {
                    case SHOW_SEQUENCE: { 
                        if (ttime > 0.2f && sequence[index] == cell_index) 
                            color = CELL_SHOWCASE_COLOR;
                    } break;
                    case USER_GUESS:
                    case USER_GUESS_WRONG:
                    case USER_GUESS_CORRECT: {
                        if (user_guess_index == cell_index) 
                            color = CELL_USER_CHOICE_COLOR;
                    } break;
                    default: 
                        break;
                    }
                    
                    DrawRectangleRec(cell_rect, color); 

                    if (state == USER_GUESS) {
                        if (is_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            user_guess_index = cell_index;
                            if (sequence[user_guess_lenght] == cell_index) {
                                user_guess_lenght += 1;
                                if (user_guess_lenght >= sequence_lenght) {
                                    state = USER_GUESS_CORRECT; 
                                    ttime = 0.0f;
                                }
                            } else {
                                ttime = 0.0f;
                                state = USER_GUESS_WRONG;
                            }
                        } 
                    }
                }
            }
            
            if (state == USER_WIN) {
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
