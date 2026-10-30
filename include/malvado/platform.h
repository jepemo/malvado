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

/* The raylib adapter owns one window and is used on the main thread. */
int mv_window_open(int width, int height, const char *title);
void mv_window_close(void);
int mv_window_should_close(void);
int mv_window_escape_down(void);
int mv_window_space_down(void);
void mv_window_begin(MvColor background);
void mv_window_draw_text(const char *text, int x, int y, int size, MvColor color);
void mv_window_flush(void);
void mv_window_end(void);
double mv_platform_time(void);
void mv_platform_wait(double seconds);

#ifdef __cplusplus
}
#endif
#endif
