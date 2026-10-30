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
#include <stdlib.h>

struct MvImage {
    Texture2D texture;
};

int mv_window_open(int width, int height, const char *title)
{
    if (width <= 0 || height <= 0 || !title || IsWindowReady()) return 0;
    InitWindow(width, height, title);
    if (!IsWindowReady()) return 0;
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    return 1;
}
void mv_window_close(void) { if (IsWindowReady()) CloseWindow(); }
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
void mv_window_begin(MvColor c)
{
    BeginDrawing();
    ClearBackground((Color){c.r, c.g, c.b, c.a});
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
void mv_window_end(void) { EndDrawing(); }
double mv_platform_time(void) { return GetTime(); }
void mv_platform_wait(double seconds) { WaitTime(seconds); }
