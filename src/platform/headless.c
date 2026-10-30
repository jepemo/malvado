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

#define _POSIX_C_SOURCE 199309L
#include "malvado/platform.h"
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

int mv_window_open(int width, int height, const char *title)
{
    (void)width; (void)height; (void)title;
    return 0;
}
void mv_window_close(void) {}
int mv_window_should_close(void) { return 1; }
int mv_window_escape_down(void) { return 0; }
int mv_window_space_down(void) { return 0; }
int mv_key_down(MvKey key) { (void)key; return 0; }
void mv_window_begin(MvColor color) { (void)color; }
void mv_window_draw_text(const char *text, int x, int y, int size, MvColor color)
{
    (void)text; (void)x; (void)y; (void)size; (void)color;
}
MvImage *mv_image_load(const char *path) { (void)path; return NULL; }
void mv_image_destroy(MvImage *image) { (void)image; }
int mv_image_width(const MvImage *image) { (void)image; return 0; }
int mv_image_height(const MvImage *image) { (void)image; return 0; }
void mv_image_draw(const MvImage *image, int x, int y)
{
    (void)image; (void)x; (void)y;
}
int mv_image_draw_region(const MvImage *image, MvImageRegion region, MvImageTransform transform)
{
    (void)image; (void)region; (void)transform;
    return 0;
}
void mv_window_flush(void) {}
void mv_window_end(void) {}
double mv_platform_time(void) { return (double)clock() / CLOCKS_PER_SEC; }
void mv_platform_wait(double seconds)
{
    if (seconds <= 0) return;
#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000));
#else
    struct timespec duration = {(time_t)seconds, (long)((seconds - (time_t)seconds) * 1e9)};
    nanosleep(&duration, NULL);
#endif
}
