#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"
#include <assert.h>
#include <string.h>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 600
#define CELL_SIZE 100
#define CELL_GAP 5

#define LINES 4
#define COLUMNS 5

#define BACKGROUND_COLOR WHITE
#define CELL_COLOR GRAY
#define OPEN_CELL_COLOR BLACK


static float ellapsed_time = 0.0f;
static bool show_pair = false;


typedef struct {
    bool open;
    int value;
} Cell;

static Cell field[LINES][COLUMNS] = {0};
static Cell *picked_cells[2]      = {NULL};


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
        } else if (picked_cells[0] != NULL && picked_cells[0] != cell) {
            if (cell->value == picked_cells[0]->value) {
                cell->open = true;
                picked_cells[0]->open = true;
                picked_cells[0] = NULL;
            } else {
                picked_cells[1] = cell;
                picked_cells[0]->open = true;
                picked_cells[1]->open = true;
                show_pair = true;
            }
        } else {
            picked_cells[0] = cell;
            picked_cells[0]->open = true;
        } 
    }
}


int main(void)
{
    assert(LINES*COLUMNS % 2 == 0 && "Must be even");
            
    srand(time(NULL));

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "memeC");
    SetTargetFPS(60);
    
    init_field();
    field_change_open_value(true);
    bool show_field = true;
    //size_t lifes = 3;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            picked_cells[0] = NULL;
            picked_cells[1] = NULL;
            show_field = true;
            ellapsed_time = 0.0f;
            init_field();
            field_change_open_value(true);
        }

        if (show_field) {
            ellapsed_time += GetFrameTime();
            if (ellapsed_time > 3.0f) {
                show_field = false;
                field_change_open_value(false);
                ellapsed_time = 0.0f;
            }
        } else if (show_pair) {
            ellapsed_time += GetFrameTime();
            if (ellapsed_time > 0.5f) {
                show_pair = false;
                picked_cells[0]->open = false;
                picked_cells[1]->open = false;
                picked_cells[0] = NULL;
                picked_cells[1] = NULL;
                ellapsed_time = 0.0f;
            }
        }
        
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            int sx = SCREEN_WIDTH/2 - (COLUMNS*CELL_SIZE + (COLUMNS-1)*CELL_GAP)/2;
            int sy = SCREEN_HEIGHT/2 - (LINES*CELL_SIZE + (LINES-1)*CELL_GAP)/2;
            
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
                    
                    if (!show_field && !show_pair) {
                        cell_event_handler(cell, is_cell_hovered);
                    } 
                }
            } 
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
