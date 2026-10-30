/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine for Lua, inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */

#include "malvado/platform.h"
#include "raylib.h"
#include "rlgl.h"
#include <math.h>
#include <limits.h>
#include <stdlib.h>

struct MvImage {
    Texture2D texture;
};
typedef struct {
    Sound original;
    size_t references;
} SharedSound;
struct MvSound {
    Sound sound;
    SharedSound *shared;
    int alias;
};
struct MvMusic {
    Music stream;
};
struct MvFont {
    Font font;
};

enum { TEXT_QUEUE_CAPACITY = 256 };
static uint32_t text_queue[TEXT_QUEUE_CAPACITY];
static size_t text_head, text_count;

int
mv_pad_connected(int slot)
{
    return slot >= 0 && slot < 4 && IsWindowReady() && IsGamepadAvailable(slot);
}

int
mv_pad_button_down(int slot, MvPadButton button)
{
    static const int buttons[] = {
        GAMEPAD_BUTTON_RIGHT_FACE_DOWN, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
        GAMEPAD_BUTTON_RIGHT_FACE_LEFT, GAMEPAD_BUTTON_RIGHT_FACE_UP,
        GAMEPAD_BUTTON_LEFT_FACE_UP, GAMEPAD_BUTTON_LEFT_FACE_DOWN,
        GAMEPAD_BUTTON_LEFT_FACE_LEFT, GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
        GAMEPAD_BUTTON_LEFT_TRIGGER_1, GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
        GAMEPAD_BUTTON_MIDDLE_LEFT, GAMEPAD_BUTTON_MIDDLE_RIGHT
    };
    if ((int)button < 0 || (size_t)button >= sizeof(buttons) / sizeof(buttons[0]) || !mv_pad_connected(slot))
        return 0;
    return IsGamepadButtonDown(slot, buttons[button]);
}

float
mv_pad_axis(int slot, MvPadAxis axis)
{
    static const int axes[] = { GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y,
                                GAMEPAD_AXIS_RIGHT_X, GAMEPAD_AXIS_RIGHT_Y,
                                GAMEPAD_AXIS_LEFT_TRIGGER, GAMEPAD_AXIS_RIGHT_TRIGGER };
    if ((int)axis < 0 || (size_t)axis >= sizeof(axes) / sizeof(axes[0]) || !mv_pad_connected(slot))
        return 0;
    if (GetGamepadAxisCount(slot) <= axes[axis])
        return 0;
    float value = GetGamepadAxisMovement(slot, axes[axis]);
    if (!isfinite(value))
        return 0;
    if (value < -1) value = -1;
    if (value > 1) value = 1;
    return axis == MV_PAD_LEFT_TRIGGER || axis == MV_PAD_RIGHT_TRIGGER ? (value + 1) / 2 : value;
}

int
mv_audio_open(void)
{
    if (IsAudioDeviceReady())
        return 1;
    InitAudioDevice();
    return IsAudioDeviceReady();
}

void
mv_audio_close(void)
{
    if (IsAudioDeviceReady())
        CloseAudioDevice();
}

MvSound *
mv_sound_load(const char *path)
{
    if (!path || !IsAudioDeviceReady() || !IsFileExtension(path, ".wav"))
        return NULL;
    Sound loaded = LoadSound(path);
    if (!IsSoundValid(loaded))
        return NULL;
    MvSound *sound = malloc(sizeof(*sound));
    SharedSound *shared = malloc(sizeof(*shared));
    if (!sound || !shared) {
        free(sound);
        free(shared);
        UnloadSound(loaded);
        return NULL;
    }
    sound->sound = loaded;
    sound->shared = shared;
    sound->alias = 0;
    *shared = (SharedSound){loaded, 1};
    return sound;
}

MvSound *
mv_sound_clone(const MvSound *sound)
{
    if (!sound || !IsAudioDeviceReady() || sound->shared->references == SIZE_MAX)
        return NULL;
    MvSound *copy = malloc(sizeof(*copy));
    if (!copy)
        return NULL;
    Sound alias = LoadSoundAlias(sound->shared->original);
    if (!IsSoundValid(alias)) {
        free(copy);
        return NULL;
    }
    sound->shared->references++;
    *copy = (MvSound){alias, sound->shared, 1};
    return copy;
}

void
mv_sound_destroy(MvSound *sound)
{
    if (!sound)
        return;
    SharedSound *shared = sound->shared;
    if (sound->alias)
        UnloadSoundAlias(sound->sound);
    else
        StopSound(sound->sound);
    if (--shared->references == 0) {
        UnloadSound(shared->original);
        free(shared);
    }
    free(sound);
}

int
mv_sound_play(MvSound *sound)
{
    if (!sound || !IsAudioDeviceReady())
        return 0;
    PlaySound(sound->sound);
    return 1;
}

int
mv_sound_stop(MvSound *sound)
{
    if (!sound || !IsAudioDeviceReady())
        return 0;
    StopSound(sound->sound);
    return 1;
}

int
mv_sound_pause(MvSound *sound)
{
    if (!sound || !IsAudioDeviceReady())
        return 0;
    PauseSound(sound->sound);
    return 1;
}

int
mv_sound_resume(MvSound *sound)
{
    if (!sound || !IsAudioDeviceReady())
        return 0;
    ResumeSound(sound->sound);
    return 1;
}

int
mv_sound_playing(const MvSound *sound)
{
    return sound && IsAudioDeviceReady() && IsSoundPlaying(sound->sound);
}

int
mv_sound_set_volume(MvSound *sound, float volume)
{
    if (!sound || !IsAudioDeviceReady() || !isfinite(volume) || volume < 0 || volume > 1)
        return 0;
    SetSoundVolume(sound->sound, volume);
    return 1;
}

MvMusic *
mv_music_load(const char *path, int loop)
{
    if (!path || !IsAudioDeviceReady() || (loop != 0 && loop != 1) ||
        (!IsFileExtension(path, ".wav") && !IsFileExtension(path, ".ogg")))
        return NULL;
    Music stream = LoadMusicStream(path);
    if (!IsMusicValid(stream))
        return NULL;
    MvMusic *music = malloc(sizeof(*music));
    if (!music) {
        UnloadMusicStream(stream);
        return NULL;
    }
    stream.looping = loop != 0;
    music->stream = stream;
    return music;
}

void
mv_music_destroy(MvMusic *music)
{
    if (!music)
        return;
    UnloadMusicStream(music->stream);
    free(music);
}

int
mv_music_play(MvMusic *music)
{
    if (!music || !IsAudioDeviceReady())
        return 0;
    PlayMusicStream(music->stream);
    return 1;
}

int
mv_music_update(MvMusic *music)
{
    if (!music || !IsAudioDeviceReady())
        return 0;
    UpdateMusicStream(music->stream);
    return 1;
}

int
mv_music_stop(MvMusic *music)
{
    if (!music || !IsAudioDeviceReady())
        return 0;
    StopMusicStream(music->stream);
    return 1;
}

int
mv_music_pause(MvMusic *music)
{
    if (!music || !IsAudioDeviceReady())
        return 0;
    PauseMusicStream(music->stream);
    return 1;
}

int
mv_music_resume(MvMusic *music)
{
    if (!music || !IsAudioDeviceReady())
        return 0;
    ResumeMusicStream(music->stream);
    return 1;
}

int
mv_music_playing(const MvMusic *music)
{
    return music && IsAudioDeviceReady() && IsMusicStreamPlaying(music->stream);
}

int
mv_music_set_volume(MvMusic *music, float volume)
{
    if (!music || !IsAudioDeviceReady() || !isfinite(volume) || volume < 0 || volume > 1)
        return 0;
    SetMusicVolume(music->stream, volume);
    return 1;
}

int mv_window_open(int width, int height, const char *title)
{
    if (width <= 0 || height <= 0 || !title || IsWindowReady()) return 0;
    text_head = 0;
    text_count = 0;
    InitWindow(width, height, title);
    if (!IsWindowReady()) return 0;
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    return 1;
}
int
mv_window_set_title(const char *title)
{
    if (!title || !IsWindowReady())
        return 0;
    SetWindowTitle(title);
    return 1;
}

int
mv_window_size(int *width, int *height)
{
    if (!width || !height)
        return 0;
    if (!IsWindowReady()) {
        *width = 0;
        *height = 0;
        return 0;
    }
    *width = GetScreenWidth();
    *height = GetScreenHeight();
    return 1;
}

void mv_window_close(void)
{
    if (IsWindowReady()) CloseWindow();
    text_head = 0;
    text_count = 0;
}
int mv_window_should_close(void) { return WindowShouldClose(); }
int mv_window_escape_down(void) { return mv_key_down(MV_KEY_ESCAPE); }
int mv_window_space_down(void) { return mv_key_down(MV_KEY_SPACE); }

int
mv_key_down(MvKey key)
{
    if (!IsWindowReady())
        return 0;
    int code;
    if (key >= MV_KEY_A && key <= MV_KEY_Z)
        code = KEY_A + (key - MV_KEY_A);
    else if (key >= MV_KEY_0 && key <= MV_KEY_9)
        code = KEY_ZERO + (key - MV_KEY_0);
    else {
        switch (key) {
        case MV_KEY_ESCAPE: code = KEY_ESCAPE; break;
        case MV_KEY_SPACE: code = KEY_SPACE; break;
        case MV_KEY_LEFT: code = KEY_LEFT; break;
        case MV_KEY_RIGHT: code = KEY_RIGHT; break;
        case MV_KEY_UP: code = KEY_UP; break;
        case MV_KEY_DOWN: code = KEY_DOWN; break;
        case MV_KEY_ENTER: code = KEY_ENTER; break;
        case MV_KEY_TAB: code = KEY_TAB; break;
        case MV_KEY_BACKSPACE: code = KEY_BACKSPACE; break;
        default: return 0;
        }
    }
    return IsKeyDown(code);
}

int
mv_mouse_position(int *x, int *y)
{
    if (!x || !y)
        return 0;
    if (!IsWindowReady()) {
        *x = 0;
        *y = 0;
        return 0;
    }
    *x = GetMouseX();
    *y = GetMouseY();
    return 1;
}

int
mv_mouse_down(MvMouseButton button)
{
    if (!IsWindowReady())
        return 0;
    int code;
    switch (button) {
    case MV_MOUSE_LEFT: code = MOUSE_BUTTON_LEFT; break;
    case MV_MOUSE_MIDDLE: code = MOUSE_BUTTON_MIDDLE; break;
    case MV_MOUSE_RIGHT: code = MOUSE_BUTTON_RIGHT; break;
    default: return 0;
    }
    return IsMouseButtonDown(code);
}

int
mv_text_input_next(uint32_t *codepoint)
{
    if (!codepoint || !IsWindowReady() || text_count == 0)
        return 0;
    *codepoint = text_queue[text_head];
    text_head = (text_head + 1) % TEXT_QUEUE_CAPACITY;
    --text_count;
    return 1;
}

static void
capture_text_input(void)
{
    int value;
    while ((value = GetCharPressed()) != 0) {
        uint32_t codepoint = (uint32_t)value;
        if (codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff))
            continue;
        if (text_count < TEXT_QUEUE_CAPACITY) {
            size_t end = (text_head + text_count) % TEXT_QUEUE_CAPACITY;
            text_queue[end] = codepoint;
            ++text_count;
        }
    }
}
void mv_window_begin(MvColor c)
{
    BeginDrawing();
    ClearBackground((Color){c.r, c.g, c.b, c.a});
}
int
mv_window_text_width(const char *text, int size)
{
    if (!text || size < 1 || !IsWindowReady())
        return 0;
    return MeasureText(text, size);
}

MvFont *
mv_font_load(const char *path, int base_size)
{
    uint32_t codepoints[191];
    size_t count = 0;
    for (uint32_t codepoint = 32; codepoint <= 126; ++codepoint)
        codepoints[count++] = codepoint;
    for (uint32_t codepoint = 160; codepoint <= 255; ++codepoint)
        codepoints[count++] = codepoint;
    return mv_font_load_codepoints(path, base_size, codepoints, count);
}

MvFont *
mv_font_load_codepoints(const char *path, int base_size, const uint32_t *codepoints, size_t count)
{
    if (!path || !IsWindowReady() || base_size < 1 || base_size > 256 ||
        !codepoints || count == 0 || count > 1024 ||
        (!IsFileExtension(path, ".ttf") && !IsFileExtension(path, ".otf")))
        return NULL;
    int *glyphs = malloc(count * sizeof(*glyphs));
    if (!glyphs)
        return NULL;
    for (size_t i = 0; i < count; ++i) {
        uint32_t value = codepoints[i];
        if (value == 0 || value > 0x10ffff || (value >= 0xd800 && value <= 0xdfff)) {
            free(glyphs);
            return NULL;
        }
        for (size_t j = 0; j < i; ++j) {
            if (codepoints[j] == value) {
                free(glyphs);
                return NULL;
            }
        }
        glyphs[i] = (int)value;
    }
    Font loaded = LoadFontEx(path, base_size, glyphs, (int)count);
    free(glyphs);
    if (!IsFontValid(loaded) || loaded.texture.id == 0 ||
        loaded.texture.id == GetFontDefault().texture.id) {
        if (loaded.texture.id != GetFontDefault().texture.id)
            UnloadFont(loaded);
        return NULL;
    }
    MvFont *font = malloc(sizeof(*font));
    if (!font) {
        UnloadFont(loaded);
        return NULL;
    }
    font->font = loaded;
    return font;
}

void
mv_font_destroy(MvFont *font)
{
    if (!font)
        return;
    UnloadFont(font->font);
    free(font);
}

int
mv_font_text_width(const MvFont *font, const char *text, int size)
{
    if (!font || !text || !IsWindowReady() || size < 1 || size > 1024)
        return 0;
    float width = MeasureTextEx(font->font, text, (float)size, size / 10.0f).x;
    if (!isfinite(width) || width < 0 || (double)width > INT_MAX)
        return 0;
    return (int)ceilf(width);
}

int
mv_font_draw(const MvFont *font, const char *text, int x, int y, int size, MvColor color)
{
    if (!font || !text || !IsWindowReady() || size < 1 || size > 1024)
        return 0;
    DrawTextEx(font->font, text, (Vector2){(float)x, (float)y}, (float)size, size / 10.0f,
               (Color){color.r, color.g, color.b, color.a});
    return 1;
}

int
mv_window_draw_box(int x, int y, int width, int height, int filled, MvColor color)
{
    if (!IsWindowReady() || width <= 0 || height <= 0 || (filled != 0 && filled != 1) ||
        (int64_t)x + width > INT_MAX || (int64_t)y + height > INT_MAX)
        return 0;
    Color c = {color.r, color.g, color.b, color.a};
    if (filled)
        DrawRectangle(x, y, width, height, c);
    else
        DrawRectangleLines(x, y, width, height, c);
    return 1;
}

int
mv_window_draw_circle(int x, int y, int radius, int filled, MvColor color)
{
    if (!IsWindowReady() || radius <= 0 || (filled != 0 && filled != 1) ||
        (int64_t)x + radius > INT_MAX || (int64_t)x - radius < INT_MIN ||
        (int64_t)y + radius > INT_MAX || (int64_t)y - radius < INT_MIN)
        return 0;
    Color c = {color.r, color.g, color.b, color.a};
    if (filled)
        DrawCircle(x, y, (float)radius, c);
    else
        DrawCircleLines(x, y, (float)radius, c);
    return 1;
}

void mv_window_draw_text(const char *text, int x, int y, int size, MvColor c)
{
    DrawText(text, x, y, size, (Color){c.r, c.g, c.b, c.a});
}

MvImage *
mv_image_load(const char *path)
{
    if (!path || !IsWindowReady() || !IsFileExtension(path, ".png"))
        return NULL;
    Image source = LoadImage(path);
    if (!IsImageValid(source))
        return NULL;
    Texture2D texture = LoadTextureFromImage(source);
    UnloadImage(source);
    if (!IsTextureValid(texture))
        return NULL;
    MvImage *image = malloc(sizeof(*image));
    if (!image) {
        UnloadTexture(texture);
        return NULL;
    }
    image->texture = texture;
    return image;
}

void
mv_image_destroy(MvImage *image)
{
    if (!image)
        return;
    UnloadTexture(image->texture);
    free(image);
}

int
mv_image_width(const MvImage *image)
{
    return image ? image->texture.width : 0;
}

int
mv_image_height(const MvImage *image)
{
    return image ? image->texture.height : 0;
}

void
mv_image_draw(const MvImage *image, int x, int y)
{
    if (image)
        DrawTexture(image->texture, x, y, WHITE);
}

int
mv_image_draw_region(const MvImage *image, MvImageRegion region, MvImageTransform transform)
{
    if (!mv_image_region_valid(image, region, transform))
        return 0;
    Rectangle source = {(float)region.x, (float)region.y, (float)region.width, (float)region.height};
    Rectangle destination = {transform.x, transform.y,
                             region.width * transform.scale, region.height * transform.scale};
    Vector2 pivot = {transform.pivot_x * transform.scale, transform.pivot_y * transform.scale};
    DrawTexturePro(image->texture, source, destination, pivot, transform.rotation, WHITE);
    return 1;
}
void mv_window_flush(void) { rlDrawRenderBatchActive(); }
void mv_window_end(void)
{
    EndDrawing();
    capture_text_input();
}
double mv_platform_time(void) { return GetTime(); }
void mv_platform_wait(double seconds) { WaitTime(seconds); }
