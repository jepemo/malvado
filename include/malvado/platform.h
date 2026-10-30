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
typedef enum {
    MV_KEY_0 = '0', MV_KEY_1, MV_KEY_2, MV_KEY_3, MV_KEY_4,
    MV_KEY_5, MV_KEY_6, MV_KEY_7, MV_KEY_8, MV_KEY_9,
    MV_KEY_A = 'A', MV_KEY_B, MV_KEY_C, MV_KEY_D, MV_KEY_E, MV_KEY_F,
    MV_KEY_G, MV_KEY_H, MV_KEY_I, MV_KEY_J, MV_KEY_K, MV_KEY_L, MV_KEY_M,
    MV_KEY_N, MV_KEY_O, MV_KEY_P, MV_KEY_Q, MV_KEY_R, MV_KEY_S, MV_KEY_T,
    MV_KEY_U, MV_KEY_V, MV_KEY_W, MV_KEY_X, MV_KEY_Y, MV_KEY_Z,
    MV_KEY_ESCAPE = 256, MV_KEY_SPACE, MV_KEY_LEFT, MV_KEY_RIGHT,
    MV_KEY_UP, MV_KEY_DOWN, MV_KEY_ENTER, MV_KEY_TAB, MV_KEY_BACKSPACE
} MvKey;
typedef enum { MV_MOUSE_LEFT = 1, MV_MOUSE_MIDDLE, MV_MOUSE_RIGHT } MvMouseButton;
typedef struct MvImage MvImage;
typedef struct { int x, y, width, height; } MvImageRegion;
typedef struct {
    float x, y;
    float pivot_x, pivot_y;
    float scale, rotation;
} MvImageTransform;

/* The raylib adapter owns one window and is used on the main thread. */
int mv_window_open(int width, int height, const char *title);
void mv_window_close(void);
int mv_window_should_close(void);
int mv_window_escape_down(void);
int mv_window_space_down(void);
/* Parse a supported lowercase key name. Return 1 on success. */
int mv_key_from_name(const char *name, MvKey *key);
/* Return 1 while the key is held. Return 0 for an unsupported key or no window. */
int mv_key_down(MvKey key);
/* Parse "left", "middle", or "right". Return 1 on success. */
int mv_mouse_button_from_name(const char *name, MvMouseButton *button);
/* Write window-relative pixel coordinates. Return 0 and write zeros if no
 * window is open. Return 0 without writing if either pointer is NULL. */
int mv_mouse_position(int *x, int *y);
/* Return 1 while the button is held, or 0 for an invalid button or no window. */
int mv_mouse_down(MvMouseButton button);
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
/* Region and pivot use source pixels. Position uses window pixels; rotation
 * is clockwise in degrees. Scale must be positive. Returns 1 if valid. */
int mv_image_region_valid(const MvImage *image, MvImageRegion region, MvImageTransform transform);
/* Draw between begin and end. Returns 0 for invalid arguments. */
int mv_image_draw_region(const MvImage *image, MvImageRegion region, MvImageTransform transform);
void mv_window_flush(void);
void mv_window_end(void);
double mv_platform_time(void);
void mv_platform_wait(double seconds);

#ifdef __cplusplus
}
#endif
#endif
