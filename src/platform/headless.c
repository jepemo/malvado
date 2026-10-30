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
int mv_window_set_title(const char *title) { (void)title; return 0; }
int mv_window_size(int *width, int *height)
{
    if (!width || !height)
        return 0;
    *width = 0;
    *height = 0;
    return 0;
}
int mv_window_should_close(void) { return 1; }
int mv_window_escape_down(void) { return 0; }
int mv_window_space_down(void) { return 0; }
int mv_key_down(MvKey key) { (void)key; return 0; }
int mv_mouse_position(int *x, int *y)
{
    if (!x || !y)
        return 0;
    *x = 0;
    *y = 0;
    return 0;
}
int mv_mouse_down(MvMouseButton button) { (void)button; return 0; }
int mv_pad_connected(int slot) { (void)slot; return 0; }
int mv_pad_button_down(int slot, MvPadButton button) { (void)slot; (void)button; return 0; }
float mv_pad_axis(int slot, MvPadAxis axis) { (void)slot; (void)axis; return 0; }
int mv_text_input_next(uint32_t *codepoint) { (void)codepoint; return 0; }
int mv_audio_open(void) { return 0; }
void mv_audio_close(void) {}
MvSound *mv_sound_load(const char *path) { (void)path; return NULL; }
MvSound *mv_sound_clone(const MvSound *sound) { (void)sound; return NULL; }
void mv_sound_destroy(MvSound *sound) { (void)sound; }
int mv_sound_play(MvSound *sound) { (void)sound; return 0; }
int mv_sound_stop(MvSound *sound) { (void)sound; return 0; }
int mv_sound_pause(MvSound *sound) { (void)sound; return 0; }
int mv_sound_resume(MvSound *sound) { (void)sound; return 0; }
int mv_sound_playing(const MvSound *sound) { (void)sound; return 0; }
int mv_sound_set_volume(MvSound *sound, float volume) { (void)sound; (void)volume; return 0; }
MvMusic *mv_music_load(const char *path, int loop) { (void)path; (void)loop; return NULL; }
void mv_music_destroy(MvMusic *music) { (void)music; }
int mv_music_play(MvMusic *music) { (void)music; return 0; }
int mv_music_update(MvMusic *music) { (void)music; return 0; }
int mv_music_stop(MvMusic *music) { (void)music; return 0; }
int mv_music_pause(MvMusic *music) { (void)music; return 0; }
int mv_music_resume(MvMusic *music) { (void)music; return 0; }
int mv_music_playing(const MvMusic *music) { (void)music; return 0; }
int mv_music_set_volume(MvMusic *music, float volume) { (void)music; (void)volume; return 0; }
void mv_window_begin(MvColor color) { (void)color; }
void mv_window_draw_text(const char *text, int x, int y, int size, MvColor color)
{
    (void)text; (void)x; (void)y; (void)size; (void)color;
}
int mv_window_text_width(const char *text, int size) { (void)text; (void)size; return 0; }
int mv_window_draw_box(int x, int y, int width, int height, int filled, MvColor color)
{
    (void)x; (void)y; (void)width; (void)height; (void)filled; (void)color;
    return 0;
}
int mv_window_draw_circle(int x, int y, int radius, int filled, MvColor color)
{
    (void)x; (void)y; (void)radius; (void)filled; (void)color;
    return 0;
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
