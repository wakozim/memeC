#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"
#include <assert.h>
#include <string.h>

#define CELL_SIZE 100
#define CELL_GAP 5

#define LINES 4
#define COLUMNS 5

#define BACKGROUND_COLOR WHITE
#define CELL_COLOR GRAY
#define OPEN_CELL_COLOR BLACK

typedef enum {
    STATE_NONE,
    STATE_SHOW_FIELD,
    STATE_SHOW_PAIR,
    STATE_USER_TURN,
    STATE_USER_WIN,
} GameState;


static float elapsed_time = 0.0f;
static float ttime = 0.0f;
static GameState state = STATE_NONE;

typedef struct {
    bool open;
    int value;
} Cell;

static Cell field[LINES][COLUMNS] = {0};

typedef struct Pair {
    Cell *first;
    Cell *second;
} Pair;

static Pair picked_cells = {0};

int random_number(int min, int max)
{
    return (rand() % (max - min)) + min;
}

int get_rand_cell(void)
{
    Cell *cell = NULL;
    int cell_index;

    do {
        cell_index = random_number(0, LINES*COLUMNS);
        cell = &field[cell_index / COLUMNS][cell_index % COLUMNS];
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
            field[y][x].value = 0;
            field[y][x].open  = false;
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
        field[first_cell / COLUMNS][first_cell % COLUMNS].value = value + 1;
        field[second_cell / COLUMNS][second_cell % COLUMNS].value = value + 1;
        values[value] = true;
    }
}

void field_change_open_value(bool new_open_value)
{
    for (int line = 0; line < LINES; line++) {
        for (int column = 0; column < COLUMNS; column++) {
            field[line][column].open = new_open_value;
        }
    }
}


void cell_event_handler(Cell *cell, bool is_cell_hovered)
{
    if (is_cell_hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        if (cell->open) {
            return;
        } else if (picked_cells.first == NULL) {
            picked_cells.first = cell;
            picked_cells.first->open = true;
        } else if (picked_cells.first != cell) {
            if (cell->value == picked_cells.first->value) {
                cell->open = true;
                picked_cells.first->open = true;
                picked_cells.first = NULL;
            } else {
                picked_cells.second = cell;
                picked_cells.first->open = true;
                picked_cells.second->open = true;
                state = STATE_SHOW_PAIR;
            }
        }
    }
}


void init_pairs_game(void)
{
    assert(LINES*COLUMNS % 2 == 0 && "Must be even");

    srand(time(NULL));

    elapsed_time = 0.0f;
    ttime = 0.0f;
    state = STATE_SHOW_FIELD;

    picked_cells.first  = NULL;
    picked_cells.second = NULL;

    init_field();
    field_change_open_value(true);
    //size_t lifes = 3;
}

void draw_pairs_screen(void)
{
    if (IsKeyPressed(KEY_R)) {
        picked_cells.first  = NULL;
        picked_cells.second = NULL;
        state = STATE_SHOW_FIELD;
        ttime = 0.0f;
        init_field();
        field_change_open_value(true);
    }

    if (state == STATE_SHOW_FIELD) {
        ttime += GetFrameTime();
        if (ttime > 3.0f) {
            state = STATE_USER_TURN;
            field_change_open_value(false);
            ttime = 0.0f;
        }
    } else if (state == STATE_SHOW_PAIR) {
        ttime += GetFrameTime();
        if (ttime > 0.5f) {
            state = STATE_USER_TURN;
            picked_cells.first->open = false;
            picked_cells.second->open = false;
            picked_cells.first = NULL;
            picked_cells.second = NULL;
            ttime = 0.0f;
        }
    }

    ClearBackground(BACKGROUND_COLOR);
    int sx = GetScreenWidth()/2 - (COLUMNS*CELL_SIZE + (COLUMNS-1)*CELL_GAP)/2;
    int sy = GetScreenHeight()/2 - (LINES*CELL_SIZE + (LINES-1)*CELL_GAP)/2;

    for (int line = 0; line < LINES; line++) {
        for (int column = 0; column < COLUMNS; column++) {
            Cell *cell = &field[line][column];

            int x = sx + (column*CELL_SIZE) + (column*CELL_GAP);
            int y = sy + (line*CELL_SIZE) + (line*CELL_GAP);
            Rectangle cell_rect = {x, y, CELL_SIZE, CELL_SIZE};
            bool is_cell_hovered = CheckCollisionPointRec(GetMousePosition(), cell_rect);

            Color color = cell->open ? OPEN_CELL_COLOR : CELL_COLOR;
            DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, color);

            if (cell->open) {
                int font_size = 50;
                const char *text = TextFormat("%d", cell->value);
                int text_size = MeasureText(text, font_size);
                int tx = (x + CELL_SIZE/2) - text_size/2;
                int ty = (y + CELL_SIZE/2) - font_size/2;
                DrawText(text, tx, ty, font_size, RED);
            }

            if (state == STATE_USER_TURN) {
                cell_event_handler(cell, is_cell_hovered);
            }
        }
    }
    elapsed_time += GetFrameTime();
}

// TODO: Show time elapsed time at the end
