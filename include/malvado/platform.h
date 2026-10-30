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

#ifndef MALVADO_PLATFORM_H
#define MALVADO_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { unsigned char r, g, b, a; } MvColor;
typedef struct MvImage MvImage;

/* The raylib adapter owns one window and is used on the main thread. */
int mv_window_open(int width, int height, const char *title);
void mv_window_close(void);
int mv_window_should_close(void);
int mv_window_escape_down(void);
int mv_window_space_down(void);
void mv_window_begin(MvColor background);
void mv_window_draw_text(const char *text, int x, int y, int size, MvColor color);
/* Load a PNG after opening a window. Returns NULL on failure.
 * The caller owns the image and must destroy it before closing the window. */
MvImage *mv_image_load(const char *path);
void mv_image_destroy(MvImage *image);
int mv_image_width(const MvImage *image);
int mv_image_height(const MvImage *image);
/* Draw the image at its top-left position between begin and end. */
void mv_image_draw(const MvImage *image, int x, int y);
void mv_window_flush(void);
void mv_window_end(void);
double mv_platform_time(void);
void mv_platform_wait(double seconds);

#ifdef __cplusplus
}
#endif
#endif
