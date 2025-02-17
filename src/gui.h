#ifndef GUI_H_
#define GUI_H_

#include "raylib.h"
#include <assert.h>

#ifndef GUI_ASSERT
#define GUI_ASSERT assert 
#endif // GUI_ASSERT

#define DA_INIT_CAP 256
#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            GUI_ASSERT((da)->items != NULL && "Buy more RAM lol");                   \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

typedef enum LayoutOrient {
    GUI_LAYOUT_VERTICAL = 0,
    GUI_LAYOUT_HORIZONTAL,
} LayoutOrient;

typedef struct Layout {
    LayoutOrient orient;
    Rectangle rect;
    int gap; 
    size_t count;
    size_t i;
} Layout;

typedef struct LayoutStack {
    Layout *items;
    size_t count;
    size_t capacity;
} LayoutStack;

static LayoutStack default_layout_stack = {0};


Rectangle layout_slot_loc(Layout *l, const char *file_path, int line, int slots_number);
void layout_stack_push(LayoutStack *ls, LayoutOrient orient, Rectangle rect, size_t count, float gap, float margin);
#define layout_stack_slot(ls, i) (GUI_ASSERT((ls)->count > 0), layout_slot_loc(&(ls)->items[(ls)->count - 1], __FILE__, __LINE__, (i)))
#define layout_stack_pop(ls) do { GUI_ASSERT((ls)->count > 0); (ls)->count -= 1; } while (0)
#define layout_begin(orient, rect, count, gap, margin) layout_stack_push(&default_layout_stack, (orient), (rect), (count), (gap), (margin))
#define layout_end() layout_stack_pop(&default_layout_stack)
#define layout_slot() layout_stack_slot(&default_layout_stack, 1)
#define layout_slot_ex(i) layout_stack_slot(&default_layout_stack, (i))

Rectangle screen_rect(void);
Rectangle fit_square(Rectangle r);
Rectangle gui_fit_rect(Rectangle source, Rectangle dest);
void gui_draw_text_centered(Font font, char *text, Rectangle boundary, float font_size, float spacing, Color tint);

#endif // GUI_H_


#ifdef GUI_IMPLEMENTATION


Rectangle layout_slot_loc(Layout *l, const char *file_path, int line, int slots_number)
{
    if (l->i + slots_number > l->count) {
        fprintf(stderr, "%s:%d: ERROR: Layout overflow\n", file_path, line);
        exit(1);
    }

    Rectangle r = {0};

    switch (l->orient) {
        case GUI_LAYOUT_HORIZONTAL: {
            float slot_width = (l->rect.width - l->gap*(l->count - 1))/l->count;
            r.width = (slot_width)*slots_number + l->gap*(slots_number-1);
            r.height = l->rect.height;
            r.x = l->rect.x + l->i*(slot_width + l->gap);
            r.y = l->rect.y;
        } break;

        case GUI_LAYOUT_VERTICAL: {
            float slot_height = (l->rect.height - l->gap*(l->count - 1))/l->count;
            r.width = l->rect.width;
            r.height = (slot_height)*slots_number + l->gap*(slots_number-1);
            r.x = l->rect.x;
            r.y = l->rect.y + l->i*(slot_height + l->gap);
        } break;

        default:
            GUI_ASSERT(0 && "Unreachable");
    }

    l->i += slots_number;

    return r;
}

void layout_stack_push(LayoutStack *ls, LayoutOrient orient, Rectangle rect, size_t count, float gap, float margin)
{
    Layout l = {0};
    l.orient = orient;
    l.rect = rect;
    // margin
    l.rect.x += margin;
    l.rect.y += margin;
    l.rect.width -= 2*margin;
    l.rect.height -= 2*margin;
    l.count = count;
    l.gap = gap;
    da_append(ls, l);
}


Rectangle screen_rect(void)
{
    Rectangle root = {0};
    root.width = GetRenderWidth();
    root.height = GetRenderHeight();
    return root;
}


Rectangle fit_square(Rectangle r)
{
    if (r.width < r.height) {
        return (Rectangle) {
            .x = r.x,
            .y = r.y + r.height/2 - r.width/2,
            .width = r.width,
            .height = r.width,
        };
    } else {
        return (Rectangle) {
            .x = r.x + r.width/2 - r.height/2,
            .y = r.y,
            .width = r.height,
            .height = r.height,
        };
    }
}


void gui_draw_text_centered(Font font, char *text, Rectangle boundary, float font_size, float spacing, Color tint)
{
    Vector2 text_size = MeasureTextEx(font, text, font_size, spacing);
    Vector2 position = {
        .x = boundary.x + boundary.width/2 - text_size.x/2, 
        .y = boundary.y + boundary.height/2 - text_size.y/2     
    };
    DrawTextEx(font, text, position, font_size, spacing, tint);
}

Rectangle gui_fit_rect(Rectangle outer, Rectangle inner) {
    float width_scale = outer.width / inner.width;
    float height_scale = outer.height / inner.height;

    float scale = width_scale < height_scale ? width_scale : height_scale;

    Rectangle rect = {
        .x = 0,
        .y = 0,
        .width = inner.width,
        .height = inner.height
    };

    if (inner.width > outer.width || inner.height > outer.height) {
        rect.width = inner.width * scale;
        rect.height = inner.height * scale;
    }

    rect.x = outer.x + (outer.width - rect.width) / 2;
    rect.y = outer.y + (outer.height - rect.height) / 2;

    return rect;
}

#endif // GUI_IMPLEMENTATION
