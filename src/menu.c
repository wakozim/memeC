#include "raylib.h"
#include "raymath.h"

#include "gui.h"
#include "screens.h"

// Assets
#define FONT_FILE_PATH "assets/fonts/IosevkaNerdFontMono-Regular.ttf"

// Colors
#define BACKGROUND_COLOR ColorFromHSV(0, 0.00f, 0.10f)
#define TEXT_COLOR       GREEN

// Params
#define MENU_TEXT_SIZE 60

#ifdef PLATFORM_WEB

#define SEQUENCE_VIDEO_FILE_PATH "assets/videos/sequence.mp4"
#define PAIRS_VIDEO_FILE_PATH    "assets/videos/pairs.mp4"

typedef struct Video {
    size_t id;
    size_t loaded;
    size_t playing;
} Video;

bool load_video(Video *video, const char *file_path);
void unload_video(Video *video);
void update_video(Video *video);
void draw_video(Video *video, Rectangle boundary, bool is_square);

#else

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

#define SEQUENCE_VIDEO_FILE_PATH "assets/videos/sequence.mpeg"
#define PAIRS_VIDEO_FILE_PATH    "assets/videos/pairs.mpeg"

typedef struct Video {
    plm_t *plm;
    Rectangle rect;
    Image image;
    Texture texture;
    float framerate;
    plm_frame_t *frame;
    int frames_count;
    int current_frame;
    int frame_counter;
    int delay;
    bool loaded;
    bool playing;
} Video;

bool load_video(Video *video, const char *file_path)
{
    video->loaded = false;
    video->plm = plm_create_with_filename(file_path);
    if (!video->plm) {
		fprintf(stderr, "[ERROR] Couldn't open file %s\n", file_path);
		return false;
	}
    video->loaded = true;
    video->playing = true;
    plm_set_audio_enabled(video->plm, false);
    video->rect.x = 0;
    video->rect.y = 0;
    video->rect.width = plm_get_width(video->plm);
    video->rect.height = plm_get_width(video->plm);
    video->image.width = video->rect.width;
    video->rect.height = plm_get_height(video->plm);
    video->image.height = video->rect.height;
    int components = 3;
    video->image.data = MemAlloc(video->image.height * video->image.width * components);
    video->image.mipmaps = 1;
    video->image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    video->texture = LoadTextureFromImage(video->image);
    video->framerate = plm_get_framerate(video->plm);
    video->frames_count = video->framerate * plm_get_duration(video->plm);
    video->current_frame = 0;
    video->frame_counter = 0;
    video->frame = NULL;
    video->delay = 60.0f / video->framerate;
    return true;
}

void unload_video(Video *video)
{
    video->loaded = false;
    if (video->plm) {
        plm_destroy(video->plm);
        video->plm = NULL;
	}
    if (IsTextureValid(video->texture)) {
        UnloadTexture(video->texture);
    }
}

void update_video(Video *video)
{
    if (!video->loaded) return;

    video->frame_counter++;
    if (video->frame_counter >= video->delay) {
        video->current_frame++;
        if (video->current_frame > video->frames_count) {
            video->current_frame = 0;
            plm_rewind(video->plm);
        }
        video->frame = plm_decode_video(video->plm);
        plm_frame_to_rgb(video->frame, (uint8_t *)video->image.data, video->rect.width * 3);
        UpdateTexture(video->texture, video->image.data);
        video->frame_counter = 0;
    }
}

void draw_video(Video *video, Rectangle boundary, bool is_square) 
{
    Rectangle rect = is_square ? fit_square(boundary) : gui_fit_rect(boundary, video->rect);
    DrawTexturePro(video->texture, video->rect, rect, Vector2Zero(), 0, WHITE);
}
#endif // PLATFORM_WEB

static Video pairs_video = {0};
static Video sequence_video = {0};
static int mouse_cursor = MOUSE_CURSOR_DEFAULT;
static Font font = {0};


void init_menu_game(void)
{
    if (!sequence_video.loaded) load_video(&sequence_video, SEQUENCE_VIDEO_FILE_PATH);
    if (!pairs_video.loaded) load_video(&pairs_video, PAIRS_VIDEO_FILE_PATH);
    if (!IsFontValid(font)) font = LoadFontEx(FONT_FILE_PATH, MENU_TEXT_SIZE, NULL, 0);
}


bool draw_game_button(Rectangle rect, Video *video, bool is_square, char *text)
{
    bool is_hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    layout_begin(GUI_LAYOUT_VERTICAL, rect, 4, 5, 0);
    Rectangle video_slot = layout_slot_ex(3);
    Rectangle text_bound = layout_slot();
    gui_draw_text_centered(font, text, text_bound, MENU_TEXT_SIZE, 5, WHITE);
    draw_video(video, video_slot, is_square);

    if (is_hovered) {
        mouse_cursor = MOUSE_CURSOR_POINTING_HAND;
        DrawRectangleRoundedLinesEx(rect, 0.1f, 0, 2.5f, ColorFromHSV(0, 0.0f, 0.75f));
    }

    layout_end();

    return is_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}


GameScreen draw_menu_screen(void)
{
    GameScreen result = MENU;
    mouse_cursor = MOUSE_CURSOR_DEFAULT;

    update_video(&sequence_video);
    update_video(&pairs_video);

    ClearBackground(BACKGROUND_COLOR);
    ///////////////////
    layout_begin(GUI_LAYOUT_VERTICAL, screen_rect(), 4, 10, 0);
    gui_draw_text_centered(font, "MemeC", layout_slot(), 60, 6, WHITE);
    ///////////////////
    layout_begin(GUI_LAYOUT_HORIZONTAL, layout_slot_ex(3), 2, 10, 10);
    bool pairs_button_pressed = draw_game_button(layout_slot(), &pairs_video, false, "Pairs");
    bool sequence_button_pressed = draw_game_button(layout_slot(), &sequence_video, true, "Sequence");
    layout_end();
    ///////////////////
    layout_end();
    ///////////////////

    if (IsKeyPressed(KEY_S) || sequence_button_pressed) {
        return SEQUENCE;
    } else if (IsKeyPressed(KEY_P) || pairs_button_pressed) {
        return PAIRS;
    }

    set_mouse_cursor(mouse_cursor);
    return result;
}
