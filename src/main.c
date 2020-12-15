#include "raylib.h"
#include <assert.h> /* assert() */
#include <limits.h>
#include <math.h>    /* fabsf(), cos(), sin() */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdio.h>   /* sprintf() */
#include <stdlib.h>  /* malloc(), calloc() */
#include <string.h>  /* memcpy(), strdup()*/

#include "audio.h"
#include "list.h"
#include "music_sheet.h"

#ifdef _WIN32
#define strdup _strdup
#endif

// ------ DEFINITIONS ------
#define WINDOW_TITLE "Music Tiles"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define LINE 4
#define LINE_WIDTH 100
#define MAX_MUSIC_NOTE_EACH_LINE 100
#define MARGIN_BETWEEN_LINE 10

#define MUSIC_NOTE_WIDTH LINE_WIDTH
#define MUSIC_NOTE_HEIGHT 160
#define MUSIC_NOTE_INIT_COLOR DARKGRAY
#define MUSIC_NOTE_AREA MUSIC_NOTE_WIDTH* MUSIC_NOTE_HEIGHT
#define INIT_MUSIC_NOTE_SPEED 12

#define TOUCH_BLOCK_WIDTH LINE_WIDTH
#define TOUCH_BLOCK_HEIGHT 40
#define TOUCH_BLOCK_AREA TOUCH_BLOCK_WIDTH* TOUCH_BLOCK_HEIGHT
#define TOUCH_BLOCK_MARGIN_BOTTOM 20
#define TOUCH_BLOCK_PERFECT_TOLERANCE 0.9f
#define TOUCH_BLOCK_OK_TOLERANCE 0.6f
#define TOUCH_BLOCK_BAD_TOLERANCE 0.01f
#define TOUCH_BLOCK_FONT_SIZE 20
#define TOUCH_BLOCK_INIT_COLOR LIGHTGRAY
#define TOUCH_BLOCK_FONT_COLOR GRAY
#define TOUCH_BLOCK_PERFECT_COLOR GREEN
#define TOUCH_BLOCK_OK_COLOR SKYBLUE
#define TOUCH_BLOCK_BAD_COLOR ORANGE
#define TOUCH_BLOCK_MISTOUCH_COLOR RED

#define SIDE_LINE_COLOR LIGHTGRAY
#define SIDE_LINE_WIDTH 1

#define UI_FONT_COLOR LIGHTGRAY
#define UI_FONT_SIZE 20
#define MISSED_FONT_COLOR DARKGRAY
#define MISSED_FONT_SIZE 20
#define SCORE_FONT_COLOR GRAY
#define SCORE_FONT_SIZE 22

#define UI_MARGIN 5
#define ANIMATE_TEXT_DURATION 30
// ------ END DEFINITIONS ------

// ------ MACROS ------
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
// ------ END MACROS ------

// ------ STRUCTURES ------
typedef struct MusicNote {
    float x;
    float y;
    float width;
    float height;
    Color color;
} MusicNote;

typedef struct TouchBlock {
    float x;
    float y;
    float width;
    float height;
    Color color;
    KeyboardKey key;
} TouchBlock;

typedef struct FrameRegistration {
    void (*function)(void*);
    unsigned int frame;
    void* data;
} FrameRegistration;

typedef struct TextBox {
    float x;
    float y;
    int fontsize;
    char* text;
    Color fontcolor;
} TextBox;
// ------ END STRUCTURES ------

// ------ GLOBAL VARIABLES ------
const int LEFT_MARGIN = (SCREEN_WIDTH - (LINE_WIDTH * LINE) - (MARGIN_BETWEEN_LINE * (LINE - 1))) / 2;
const int RIGHT_MARGIN = (SCREEN_WIDTH - (LINE_WIDTH * LINE) - (MARGIN_BETWEEN_LINE * (LINE - 1))) / 2;
const static KeyboardKey TOUCH_BLOCK_KEYS[LINE] = { KEY_H, KEY_J, KEY_K, KEY_L };

static TouchBlock touch_blocks[LINE] = { 0 };
static List music_note_lists[LINE] = { 0 };
static List frame_registrations = { 0 };
static List animation_texts = { 0 };
static List music_sheets = { 0 };
static ListNode* current_music_sheet_node = NULL;

static unsigned int frame_counter = 0;
static int score = 0;
static int miss = 0;
// ------ END GLOBAL VARIABLES ------

ListNode* find_highest_music_note()
{
    ListNode* minnode = NULL;
    float miny = INFINITY;
    for (int line = 0; line < LINE; ++line) {
        for (ListNode* node = music_note_lists[line].head; node != NULL; node = node->prev) {
            if (((MusicNote*)node->data)->y < miny) {
                miny = ((MusicNote*)node->data)->y;
                minnode = node;
            }
        }
    }

    return minnode;
}

float randf(float min, float max)
{
    return (min + ((float)GetRandomValue(0, INT_MAX) / (float)INT_MAX) * (max - min));
}

float rect_intersect_area(Rectangle a, Rectangle b)
{
    if ((a.x + a.width <= b.x || a.y + a.height <= b.y) || (b.x + b.width <= a.x || b.y + b.height <= a.y))
        return 0.0f;
    else
        return fabsf((MAX(a.x, b.x) - MIN(a.x + a.width, b.x + b.width)) * (MAX(a.y, b.y) - MIN(a.y + a.height, b.y + b.height)));
}

// get the music note drop speed now
float speed_now()
{
    return INIT_MUSIC_NOTE_SPEED + (frame_counter / 3000.0f);
}

// data will be copied
// run a function at a future point of frame
// RETURN: true if registration successful else false
static bool register_at_frame(unsigned int frame, void (*f)(void*), void* data, size_t data_size)
{
    if (frame_counter > frame)
        return false;

    FrameRegistration registration = {
        .function = f, .frame = frame, .data = memcpy(malloc(data_size), data, data_size)
    };
    list_append_tail(&frame_registrations, &registration, sizeof(FrameRegistration));
    return true;
}

typedef struct
{
    TouchBlock* touch_block;
    Color color;
} __frame_register_set_touch_block_color_t;
void __frame_register_set_touch_block_color(__frame_register_set_touch_block_color_t* data)
{
    data->touch_block->color = data->color;
}

typedef struct
{
    TextBox* textbox;
    float dx;
    float dy;
} __frame_register_animate_text_t;
void __frame_register_animate_text(__frame_register_animate_text_t* data)
{
    data->textbox->x += data->dx;
    data->textbox->y += data->dy;
}

typedef struct
{
    ListNode* node;
} __frame_register_drop_animate_text_t;
void __frame_register_drop_animate_text(__frame_register_drop_animate_text_t* data)
{
    TextBox* textbox = (TextBox*)list_pop_node(&animation_texts, data->node);
    free(textbox->text);
    free(textbox);
}

void register_animate_text(unsigned int frame, const char* text, int fontsize, Color fontcolor, float start_x,
    float start_y)
{
    TextBox textbox = { .x = start_x, .y = start_y, .fontsize = fontsize, .text = strdup(text), .fontcolor = fontcolor };

    list_append_tail(&animation_texts, &textbox, sizeof(TextBox));

    float radians = randf(PI / 2 - PI / 6, PI / 2 + PI / 6); // 60 ~ 120 degrees
    for (int i = 0; i < ANIMATE_TEXT_DURATION; ++i) {
        __frame_register_animate_text_t data = {
            .textbox = animation_texts.tail->data, .dx = -2 * cosf(radians), .dy = -2 * sinf(radians)
        };
        register_at_frame(frame + i, (void (*)(void*))__frame_register_animate_text, &data,
            sizeof(__frame_register_animate_text_t));
    }

    __frame_register_drop_animate_text_t _data = { .node = animation_texts.tail };
    register_at_frame(frame + ANIMATE_TEXT_DURATION, (void (*)(void*))__frame_register_drop_animate_text, &_data,
        sizeof(__frame_register_drop_animate_text_t));
}

static void InitGame(const char* sheet_files[], size_t sheet_count)
{
    // Load all the music sheets
    for (int i = 0; i < sheet_count; ++i) {
        MusicSheet sheet;
        LoadMusicSheetFromFile(&sheet, sheet_files[i]);
        list_append_tail(&music_sheets, &sheet, sizeof(MusicSheet));
    }

    if (sheet_count == 0) {
        MusicSheet sheet;
        GetDefaultMusicSheet(&sheet);
        list_append_tail(&music_sheets, &sheet, sizeof(MusicSheet));
    }

    current_music_sheet_node = music_sheets.head;

    // --- init touch blocks ---
    for (int line = 0; line < LINE; ++line) {
        touch_blocks[line].width = TOUCH_BLOCK_WIDTH;
        touch_blocks[line].height = TOUCH_BLOCK_HEIGHT;
        touch_blocks[line].x = LEFT_MARGIN + (line * TOUCH_BLOCK_WIDTH) + (line * MARGIN_BETWEEN_LINE);
        touch_blocks[line].y = SCREEN_HEIGHT - TOUCH_BLOCK_HEIGHT - TOUCH_BLOCK_MARGIN_BOTTOM;
        touch_blocks[line].color = TOUCH_BLOCK_INIT_COLOR;
        touch_blocks[line].key = TOUCH_BLOCK_KEYS[line];
    }
}

void UpdateGame()
{
    // push a new music node
    ListNode* highest_node = find_highest_music_note();
    while (highest_node == NULL || ((MusicNote*)highest_node->data)->y >= -MUSIC_NOTE_HEIGHT) {
        int line = GetRandomValue(0, LINE - 1);
        MusicNote music_note = { .x = LEFT_MARGIN + (line * LINE_WIDTH) + (line * MARGIN_BETWEEN_LINE),
            .y = (highest_node == NULL) ? -MUSIC_NOTE_HEIGHT
                                        : ((MusicNote*)highest_node->data)->y - MUSIC_NOTE_HEIGHT,
            .width = MUSIC_NOTE_WIDTH,
            .height = MUSIC_NOTE_HEIGHT,
            .color = MUSIC_NOTE_INIT_COLOR };
        list_append_tail(&music_note_lists[line], &music_note, sizeof(MusicNote));
        highest_node = music_note_lists[line].tail;
    }

    // missed music nodes (exceed screen height)
    for (int line = 0; line < LINE; ++line)
        for (ListNode* node = music_note_lists[line].head; node != NULL; node = node->prev) {
            MusicNote* music_note = ((MusicNote*)node->data);
            if (music_note->y > SCREEN_HEIGHT) {
                register_animate_text(frame_counter + 1, "Missed", MISSED_FONT_SIZE, MISSED_FONT_COLOR, music_note->x,
                    SCREEN_HEIGHT - MISSED_FONT_SIZE);
                free(list_pop_node(&music_note_lists[line], node));
                miss += 1;
                break;
            }
        }

    // player pressed touch block
    for (int line = 0; line < LINE; ++line) {
        if (IsKeyPressed(touch_blocks[line].key)) {
            bool touch_success = false;
            int score_increment = 0;
            for (ListNode* node = music_note_lists[line].head; node != NULL; node = node->prev) {
                MusicNote* music_note = (MusicNote*)node->data;
                Rectangle touch_rect = { .x = touch_blocks[line].x,
                    .y = touch_blocks[line].y,
                    .width = touch_blocks[line].width,
                    .height = touch_blocks[line].height };
                Rectangle music_note_rect = {
                    .x = music_note->x, .y = music_note->y, .width = music_note->width, .height = music_note->height
                };
                float intersect_area = rect_intersect_area(touch_rect, music_note_rect);
                if (intersect_area / MIN(TOUCH_BLOCK_AREA, MUSIC_NOTE_AREA) >= TOUCH_BLOCK_PERFECT_TOLERANCE) {
                    touch_success = true;
                    free(list_pop_node(&music_note_lists[line], node));
                    touch_blocks[line].color = TOUCH_BLOCK_PERFECT_COLOR;
                    score_increment = 5;
                    break;
                } else if (intersect_area / MIN(TOUCH_BLOCK_AREA, MUSIC_NOTE_AREA) >= TOUCH_BLOCK_OK_TOLERANCE) {
                    touch_success = true;
                    free(list_pop_node(&music_note_lists[line], node));
                    touch_blocks[line].color = TOUCH_BLOCK_OK_COLOR;
                    score_increment = 3;
                    break;
                } else if (intersect_area / MIN(TOUCH_BLOCK_AREA, MUSIC_NOTE_AREA) >= TOUCH_BLOCK_BAD_TOLERANCE) {
                    touch_success = true;
                    free(list_pop_node(&music_note_lists[line], node));
                    touch_blocks[line].color = TOUCH_BLOCK_BAD_COLOR;
                    score_increment = 1;
                    break;
                } else {
                    touch_success = false;
                    break;
                }
            }

            if (touch_success) {
                char buffer[16];
                if (MusicSheetEnded((MusicSheet*)current_music_sheet_node->data)) {
                    MusicSheetRewind((MusicSheet*)current_music_sheet_node->data);
                    if (current_music_sheet_node->prev == NULL) {
                        current_music_sheet_node = music_sheets.head;
                    } else {
                        current_music_sheet_node = current_music_sheet_node->prev;
                    }
                }

                PlayMusicSheet((MusicSheet*)current_music_sheet_node->data);

                sprintf(buffer, "+%d", score_increment);
                score += score_increment;
                register_animate_text(frame_counter + 1, buffer, SCORE_FONT_SIZE, SCORE_FONT_COLOR,
                    touch_blocks[line].x, touch_blocks[line].y);
            } else {
                touch_blocks[line].color = TOUCH_BLOCK_MISTOUCH_COLOR;
            }

            __frame_register_set_touch_block_color_t _data = { .touch_block = &touch_blocks[line],
                .color = TOUCH_BLOCK_INIT_COLOR };
            register_at_frame(frame_counter + 10, (void (*)(void*))__frame_register_set_touch_block_color,
                (void*)&_data, sizeof(__frame_register_set_touch_block_color_t));
        }
    }

    // music notes movement
    for (int line = 0; line < LINE; ++line) {
        for (ListNode* node = music_note_lists[line].head; node != NULL; node = node->prev) {
            ((MusicNote*)node->data)->y += speed_now();
        }
    }

    // run previously registered functions
    for (ListNode* node = frame_registrations.head; node != NULL;) {
        FrameRegistration* registration = (FrameRegistration*)node->data;
        assert(registration->frame >= frame_counter);
        if (frame_counter == registration->frame) {
            registration->function(registration->data);
            list_pop_node(&frame_registrations, node);
            free(registration->data);
            free(registration);
            node = frame_registrations.head;
            continue;
        }
        node = node->prev;
    }

    frame_counter++;
}

void DrawGame()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // --- all lines' sides ---
    for (int line = 0; line < LINE; ++line) {
        // left side
        DrawLineEx((Vector2) { LEFT_MARGIN + (line * LINE_WIDTH) + (line * MARGIN_BETWEEN_LINE), 0 },
            (Vector2) { LEFT_MARGIN + (line * LINE_WIDTH) + (line * MARGIN_BETWEEN_LINE),
                SCREEN_HEIGHT - TOUCH_BLOCK_MARGIN_BOTTOM },
            SIDE_LINE_WIDTH, SIDE_LINE_COLOR);

        // right side
        DrawLineEx((Vector2) { LEFT_MARGIN + LINE_WIDTH + (line * LINE_WIDTH) + (line * MARGIN_BETWEEN_LINE), 0 },
            (Vector2) { LEFT_MARGIN + LINE_WIDTH + (line * LINE_WIDTH) + (line * MARGIN_BETWEEN_LINE),
                SCREEN_HEIGHT - TOUCH_BLOCK_MARGIN_BOTTOM },
            SIDE_LINE_WIDTH, SIDE_LINE_COLOR);
    }

    // --- music nodes ---
    for (int line = 0; line < LINE; ++line)
        for (ListNode* node = music_note_lists[line].head; node != NULL; node = node->prev) {
            DrawRectangle(((MusicNote*)node->data)->x, ((MusicNote*)node->data)->y, ((MusicNote*)node->data)->width,
                ((MusicNote*)node->data)->height, ((MusicNote*)node->data)->color);
        }

    // --- touch blocks ---
    for (int line = 0; line < LINE; ++line) {
        DrawRectangle(touch_blocks[line].x, touch_blocks[line].y, touch_blocks[line].width, touch_blocks[line].height,
            touch_blocks[line].color);

        // text on touch block
        char text[2] = { touch_blocks[line].key, '\0' };
        DrawText(text, touch_blocks[line].x + TOUCH_BLOCK_FONT_SIZE / 2.0,
            touch_blocks[line].y + TOUCH_BLOCK_FONT_SIZE / 2.0, TOUCH_BLOCK_FONT_SIZE, TOUCH_BLOCK_FONT_COLOR);
    }

    // --- text animations ---
    for (ListNode* node = animation_texts.head; node != NULL; node = node->prev) {
        TextBox* textbox = (TextBox*)node->data;
        DrawText(textbox->text, textbox->x, textbox->y, textbox->fontsize, textbox->fontcolor);
    }

    // --- text of upper-left corner ---
    DrawText(TextFormat("SCORE: %d", score), UI_MARGIN, UI_MARGIN, UI_FONT_SIZE, LIGHTGRAY);
    DrawText(TextFormat("MISS: %d", miss), UI_MARGIN, UI_MARGIN + UI_FONT_SIZE, UI_FONT_SIZE, UI_FONT_COLOR);
    DrawText(TextFormat("SPEED: %.1f", speed_now()), UI_MARGIN, UI_MARGIN + UI_FONT_SIZE * 2, UI_FONT_SIZE,
        UI_FONT_COLOR);

    EndDrawing();
}

void textbox_free(TextBox* textbox)
{
    if (textbox == NULL)
        return;
    free(textbox->text);
    free(textbox);
}

void EndGame()
{
    // clean up
    for (int line = 0; line < LINE; line++)
        list_clear(&music_note_lists[line], free);

    list_clear(&animation_texts, (void (*)(void*))textbox_free);
    list_clear(&music_sheets, (void (*)(void*))MusicSheetFree);
}

int main(int argc, const char* argv[])
{
    InitAudioDevice();
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);
    InitGame(argv + 1, argc - 1);
    while (!WindowShouldClose()) {
        UpdateGame();
        DrawGame();
    }
    EndGame();
    CloseWindow();
    CloseAudioDevice();

    return 0;
}
