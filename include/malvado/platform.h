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

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { unsigned char r, g, b, a; } MvColor;
typedef struct {
    MvColor current, start, target;
    unsigned duration, elapsed;
} MvFade;
/* Initialize to {0}. Start a transition from the current color. Duration is
 * 0..1000000 ticks; zero changes the color immediately. Return 0 on invalid
 * input without changing the state. Step once per tick and draw current. */
int mv_fade_start(MvFade *fade, MvColor target, unsigned ticks);
int mv_fade_step(MvFade *fade);
int mv_fade_active(const MvFade *fade);
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
typedef enum {
    MV_PAD_A, MV_PAD_B, MV_PAD_X, MV_PAD_Y,
    MV_PAD_UP, MV_PAD_DOWN, MV_PAD_LEFT, MV_PAD_RIGHT,
    MV_PAD_LB, MV_PAD_RB, MV_PAD_BACK, MV_PAD_START
} MvPadButton;
typedef enum {
    MV_PAD_LEFT_X, MV_PAD_LEFT_Y, MV_PAD_RIGHT_X, MV_PAD_RIGHT_Y,
    MV_PAD_LEFT_TRIGGER, MV_PAD_RIGHT_TRIGGER
} MvPadAxis;
typedef struct MvImage MvImage;
typedef struct MvSound MvSound;
typedef struct MvMusic MvMusic;
typedef struct { int x, y, width, height; } MvImageRegion;
typedef struct {
    float x, y;
    float pivot_x, pivot_y;
    float scale, rotation;
} MvImageTransform;

/* The raylib adapter owns one window and is used on the main thread. */
int mv_window_open(int width, int height, const char *title);
/* Set the window title. Return 0 for NULL or when no window is open. */
int mv_window_set_title(const char *title);
/* Write window dimensions in logical pixels. Return 0 and write zeros when
 * no window is open, or return 0 without writing for a NULL output pointer. */
int mv_window_size(int *width, int *height);
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
/* Slots are 0 through 3. Input is updated when the window processes events.
 * A disconnected device, invalid input, or no window gives zero. */
int mv_pad_connected(int slot);
int mv_pad_button_down(int slot, MvPadButton button);
float mv_pad_axis(int slot, MvPadAxis axis);
int mv_pad_button_from_name(const char *name, MvPadButton *button);
int mv_pad_axis_from_name(const char *name, MvPadAxis *axis);
/* Remove one Unicode scalar from the shared text queue. Return 1 on success.
 * Return 0 without changing *codepoint if the queue is empty or the pointer
 * is NULL. The queue holds 256 characters and drops new characters when full. */
int mv_text_input_next(uint32_t *codepoint);
/* Encode one Unicode scalar into up to four bytes. Return the byte count, or
 * 0 for an invalid scalar or NULL output. The bytes are not NUL-terminated. */
size_t mv_utf8_encode(uint32_t codepoint, char output[4]);
/* Audio and sound functions use the owner thread. Open the device before
 * loading sounds. Destroy all sounds before closing the device. */
int mv_audio_open(void);
void mv_audio_close(void);
/* Load a WAV file. Return an owned sound, or NULL on failure. */
MvSound *mv_sound_load(const char *path);
/* Create an independent playback instance that shares the sound data.
 * Each instance is owned and must be destroyed before closing the device. */
MvSound *mv_sound_clone(const MvSound *sound);
void mv_sound_destroy(MvSound *sound);
/* Play restarts this sound. These functions return 0 for invalid arguments. */
int mv_sound_play(MvSound *sound);
int mv_sound_stop(MvSound *sound);
int mv_sound_pause(MvSound *sound);
int mv_sound_resume(MvSound *sound);
int mv_sound_playing(const MvSound *sound);
/* Volume must be finite and in the range 0 to 1. */
int mv_sound_set_volume(MvSound *sound, float volume);
/* Streams require regular updates on the audio owner thread. Destroy each
 * stream before closing the device. The loop argument must be 0 or 1. */
MvMusic *mv_music_load(const char *path, int loop);
void mv_music_destroy(MvMusic *music);
int mv_music_play(MvMusic *music);
int mv_music_update(MvMusic *music);
int mv_music_stop(MvMusic *music);
int mv_music_pause(MvMusic *music);
int mv_music_resume(MvMusic *music);
int mv_music_playing(const MvMusic *music);
int mv_music_set_volume(MvMusic *music, float volume);
void mv_window_begin(MvColor background);
void mv_window_draw_text(const char *text, int x, int y, int size, MvColor color);
/* Return the built-in font width in pixels, or 0 for invalid input or no window. */
int mv_window_text_width(const char *text, int size);
/* Draw a rectangle or circle between begin and end. Filled must be 0 or 1.
 * Return 0 for invalid geometry, an invalid flag, or no window. */
int mv_window_draw_box(int x, int y, int width, int height, int filled, MvColor color);
int mv_window_draw_circle(int x, int y, int radius, int filled, MvColor color);
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

/* Write monotonic elapsed-time clock seconds with an unspecified origin.
 * Return 1 on success, or 0 without changing the output on failure. */
int mv_monotonic_time(double *seconds);

#ifdef __cplusplus
}
#endif
#endif
