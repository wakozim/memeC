#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"

#include "screens.h"

#define CELL_SIZE 100
#define CELL_GAP 7

#define LINES 4
#define COLUMNS 5

#define BACKGROUND_COLOR ColorFromHSV(0, 0.00f, 0.10f)
#define OPEN_CELL_COLOR  ColorFromHSV(120, 0.50f, 0.35f)
#define CELL_COLOR       ColorFromHSV(0, 0.00f, 0.25f)
#define TEXT_COLOR       ColorFromHSV(0, 0.00f, 0.90f)

#define MAX_SHOW_PAIRS_TIME 3.0f
#define MAX_SHOW_PAIR_TIME  0.6f

typedef enum {
    STATE_NONE,
    STATE_SHOW_FIELD,
    STATE_SHOW_PAIR,
    STATE_USER_TURN,
    STATE_USER_WIN,
} State;

typedef struct {
    bool open;
    int value;
    float time;
} Cell;

typedef struct Pair {
    Cell *first;
    Cell *second;
} Pair;

typedef struct Game {
    float time;                 // Time for animations and interpolations
    State state;                // Game state
    Cell field[LINES][COLUMNS]; // Field state
    Pair picked_cells;          // Picked cells
} Game;


static Game game = {0};


int random_number(int min, int max)
{
    return min + (rand() % (max - min));
}

int get_rand_cell(void)
{
    Cell *cell = NULL;
    int cell_index;

    do {
        cell_index = random_number(0, LINES*COLUMNS);
        cell = &game.field[cell_index / COLUMNS][cell_index % COLUMNS];
    } while (cell->value != 0);

    return cell_index;
}


void get_rand_cells_pair(int *first, int *second)
{
    (*first) = get_rand_cell();

    do {
        (*second) = get_rand_cell();
    } while (*first == *second);
}

void clear_field(void)
{
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLUMNS; x++) {
            game.field[y][x].value = 0;
            game.field[y][x].time = 0.0f;
            game.field[y][x].open  = false;
        }
    }
}

void field_change_open_value(bool new_open_value)
{
    for (int line = 0; line < LINES; line++) {
        for (int column = 0; column < COLUMNS; column++) {
            game.field[line][column].open = new_open_value;
        }
    }
}

void init_field(void)
{
    clear_field();
    bool values[(LINES*COLUMNS)/2] = {false};
    for (int i = 0; i < (LINES*COLUMNS)/2; i++) {
        int first_cell, second_cell;
        get_rand_cells_pair(&first_cell, &second_cell);
        int value = random_number(0, (LINES*COLUMNS)/2);
        while (values[value]) {
            value = random_number(0, (LINES*COLUMNS)/2);
        }
        game.field[first_cell / COLUMNS][first_cell % COLUMNS].value = value + 1;
        game.field[second_cell / COLUMNS][second_cell % COLUMNS].value = value + 1;
        values[value] = true;
    }
}


void restart_pairs_game(void)
{
    assert(LINES*COLUMNS % 2 == 0 && "Must be even");

    game.picked_cells.first  = NULL;
    game.picked_cells.second = NULL;

    game.time = MAX_SHOW_PAIRS_TIME;

    game.state = STATE_SHOW_FIELD;

    init_field();
    field_change_open_value(true);
}


void init_pairs_game(void)
{
    restart_pairs_game();
}


void cell_event_handler(Cell *cell, bool is_cell_hovered)
{
    if (is_cell_hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        if (cell->open) {
            return;
        } else if (game.picked_cells.first == NULL) {
            game.picked_cells.first = cell;
            game.picked_cells.first->open = true;
        } else if (game.picked_cells.first != cell) {
            if (cell->value == game.picked_cells.first->value) {
                cell->open = true;
                game.picked_cells.first->open = true;
                game.picked_cells.first = NULL;
            } else {
                game.picked_cells.second = cell;
                game.picked_cells.first->open = true;
                game.picked_cells.second->open = true;
                game.state = STATE_SHOW_PAIR;
                game.time = MAX_SHOW_PAIR_TIME;
            }
        }
    }
}


void draw_pairs_field(void)
{
    int sx = GetScreenWidth()/2 - (COLUMNS*CELL_SIZE + (COLUMNS-1)*CELL_GAP)/2;
    int sy = GetScreenHeight()/2 - (LINES*CELL_SIZE + (LINES-1)*CELL_GAP)/2;

    for (int line = 0; line < LINES; line++) {
        for (int column = 0; column < COLUMNS; column++) {
            Cell *cell = &game.field[line][column];

            int x = sx + (column*CELL_SIZE) + (column*CELL_GAP);
            int y = sy + (line*CELL_SIZE) + (line*CELL_GAP);
            Rectangle cell_rect = {x, y, CELL_SIZE, CELL_SIZE};
            bool is_cell_hovered = CheckCollisionPointRec(GetMousePosition(), cell_rect);
            if (!cell->open && is_cell_hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                int cell_size = CELL_SIZE - 10;
                x = x + CELL_SIZE/2 - cell_size/2;
                y = y + CELL_SIZE/2 - cell_size/2;
                cell_rect = (Rectangle){x, y, cell_size, cell_size};
            }

            Color color = CELL_COLOR;
            if (cell->open) color = OPEN_CELL_COLOR;
            else if (is_cell_hovered) color = ColorBrightness(CELL_COLOR, 0.25f);

            DrawRectangleRounded(cell_rect, 0.1, 0, color);

            if (cell->open) {
                int font_size = 50;
                const char *text = TextFormat("%d", cell->value);
                int text_size = MeasureText(text, font_size);
                int tx = (x + CELL_SIZE/2) - text_size/2;
                int ty = (y + CELL_SIZE/2) - font_size/2;
                DrawText(text, tx, ty, font_size, TEXT_COLOR);
            }

            if (game.state == STATE_USER_TURN) {
                cell_event_handler(cell, is_cell_hovered);
            }
        }
    }
}


GameScreen draw_pairs_screen(void)
{
    GameScreen result = PAIRS;

    ClearBackground(BACKGROUND_COLOR);
    draw_pairs_field();

    if (game.time >= 0.0f) game.time -= GetFrameTime();

    if (IsKeyPressed(KEY_R)) {
        restart_pairs_game();
    } else if (IsKeyPressed(KEY_Q)) {
        result = MENU;
    }

    switch (game.state) {
        case STATE_SHOW_FIELD: {
            if (game.time > 0.0f) break;
            game.state = STATE_USER_TURN;
            field_change_open_value(false);
        } break;
        case STATE_SHOW_PAIR: {
            if (game.time > 0.0f) break;
            game.state = STATE_USER_TURN;
            game.picked_cells.first->open = false;
            game.picked_cells.second->open = false;
            game.picked_cells.first = NULL;
            game.picked_cells.second = NULL;
        } break;
    }

    return result;
}
