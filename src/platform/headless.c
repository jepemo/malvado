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
void mv_window_begin(MvColor color) { (void)color; }
void mv_window_draw_text(const char *text, int x, int y, int size, MvColor color)
{
    (void)text; (void)x; (void)y; (void)size; (void)color;
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
