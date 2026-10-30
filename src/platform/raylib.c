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
int mv_window_escape_down(void) { return IsKeyDown(KEY_ESCAPE); }
int mv_window_space_down(void) { return IsKeyDown(KEY_SPACE); }
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
void mv_window_flush(void) { rlDrawRenderBatchActive(); }
void mv_window_end(void) { EndDrawing(); }
double mv_platform_time(void) { return GetTime(); }
void mv_platform_wait(double seconds) { WaitTime(seconds); }
