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

/* A copied, backend-independent FIFO. Initialize to {0}; do not modify its
 * fields except through these operations. One active transition and at most
 * 16 pending transitions. Read fade.current for the current overlay color. */
typedef struct { MvColor target; unsigned ticks; } MvFadeTransition;
typedef struct {
    MvFade fade;
    MvFadeTransition pending[16];
    unsigned head, count;
} MvFadeQueue;
/* Return 1 on success, 0 for NULL or duration > 1000000, -1 for a full queue.
 * Failure changes nothing. An idle queue starts immediately, including a
 * zero-tick change. A busy queue appends, including zero-tick changes. */
int mv_fade_queue_push(MvFadeQueue *queue, MvColor target, unsigned ticks);
/* Advance the active transition once, then start pending transitions in FIFO
 * order. Drain zero-tick transitions on that same step. A newly started timed
 * transition first advances on the next step. Return 1 if state changed, or 0
 * for NULL or idle. Call once per simulation tick, independent of rendering. */
int mv_fade_queue_step(MvFadeQueue *queue);
/* Return 1 while a timed transition or pending request remains, otherwise 0. */
int mv_fade_queue_active(const MvFadeQueue *queue);

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
typedef struct MvFont MvFont;
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
/* Parse a supported lowercase key name. Return 1 on success, or 0 without
 * changing the output for an invalid name or NULL argument. No window is needed. */
int mv_key_from_name(const char *name, MvKey *key);
/* Return 1 while the key is held. Return 0 for an unsupported key or no window. */
int mv_key_down(MvKey key);
/* Commit buffered keyboard transitions once before each logical simulation
 * tick, not once per process. mv_window_end captures platform events. Frames
 * without ticks retain pending events; extra catch-up ticks see no repeat.
 * The Lua runner calls this automatically. Use on the window owner thread. */
void mv_input_tick(void);
/* Non-consuming transition snapshots for the current logical tick. Return 0
 * for unsupported keys or no window. Both can be true if press and release
 * were observed between ticks. These are not OS autorepeat or text events. */
int mv_key_pressed(MvKey key);
int mv_key_released(MvKey key);
/* Parse "left", "middle", or "right". Return 1 on success, or 0 without
 * changing the output for an invalid name or NULL argument. No window is needed. */
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
/* Parse a case-sensitive control name. Return 1 on success, or 0 without
 * changing the output for an invalid name or NULL argument. No window is needed. */
int mv_pad_button_from_name(const char *name, MvPadButton *button);
int mv_pad_axis_from_name(const char *name, MvPadAxis *axis);
/* Remove one Unicode scalar from the shared text queue. Return 1 on success.
 * Return 0 without changing *codepoint if the queue is empty or the pointer
 * is NULL. The queue holds 256 characters and drops new characters when full. */
int mv_text_input_next(uint32_t *codepoint);
/* Encode one Unicode scalar into up to four bytes. Return the byte count, or
 * 0 for an invalid scalar or NULL output. The bytes are not NUL-terminated. */
size_t mv_utf8_encode(uint32_t codepoint, char output[4]);
/* Decode one Unicode scalar. Return bytes consumed, or 0 for invalid input
 * without changing the output. The input need not be NUL-terminated. */
size_t mv_utf8_decode(const char *input, size_t length, uint32_t *codepoint);
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
/* Clip subsequent drawing to a rectangle inside the logical window. Call
 * between window_begin and window_end. Rectangles must be nonempty and fully
 * inside the window. Nested clips are rejected. Invalid calls leave the clip
 * unchanged. clip_end restores unrestricted drawing. window_end also clears
 * an active clip. Return 1 on success, 0 on invalid state or arguments. */
int mv_window_clip_begin(int x, int y, int width, int height);
int mv_window_clip_end(void);
/* Translate world drawing by (-x, -y) within the current frame. Coordinates
 * must be finite and within FLT_MAX / 16. Start any clip before scrolling;
 * nesting scrolls or changing the clip while scrolling is rejected. Ending a
 * frame restores both states. Return 1 on success, 0 on invalid input/state. */
int mv_window_scroll_begin(float x, float y);
int mv_window_scroll_end(void);
void mv_window_draw_text(const char *text, int x, int y, int size, MvColor color);
/* Return the built-in font width in pixels, or 0 for invalid input, no window,
 * or a width that cannot fit in an int. */
int mv_window_text_width(const char *text, int size);
/* Load a TTF/OTF font after opening a window. The caller owns the font and
 * must destroy it before closing the window. The initial glyph set covers
 * printable ASCII and Latin-1. Base size must be in 1..256. */
MvFont *mv_font_load(const char *path, int base_size);
/* Load 1..1024 distinct Unicode scalars as the font glyph set. Code point 0
 * and surrogate values are invalid. The input array is borrowed. */
MvFont *mv_font_load_codepoints(const char *path, int base_size,
                                const uint32_t *codepoints, size_t count);
void mv_font_destroy(MvFont *font);
int mv_font_text_width(const MvFont *font, const char *text, int size);
int mv_font_draw(const MvFont *font, const char *text, int x, int y, int size, MvColor color);
/* Draw a rectangle or circle between begin and end. Filled must be 0 or 1.
 * Return 0 for invalid geometry, an invalid flag, or no window. */
int mv_window_draw_box(int x, int y, int width, int height, int filled, MvColor color);
int mv_window_draw_circle(int x, int y, int radius, int filled, MvColor color);
/* Filled shapes at floating-point coordinates. Rectangles rotate clockwise
 * around the supplied pivot, in local pixels. Return 0 for invalid input. */
int mv_window_draw_transformed_rect(float x, float y, float width, float height,
                                    float pivot_x, float pivot_y, float angle, MvColor color);
int mv_window_draw_filled_circle(float x, float y, float radius, MvColor color);
/* Load a PNG after opening a window. Returns NULL on failure. Repeated loads
 * of the same resolved absolute path share the live image/texture. Each success
 * returns one owned reference: destroy it once per load, before window close.
 * The weak cache releases the texture with its last reference. Live images do
 * not reload when their source file changes. RGBA-created images are not cached. */
MvImage *mv_image_load(const char *path);
/* Create an image from packed RGBA bytes. Input is borrowed only during the
 * call. Width and height are positive, at most 8192, and length must cover
 * width * height * 4 bytes. Requires an open graphics window. Returns an
 * owned image or NULL; destroy it before closing the window. */
MvImage *mv_image_from_rgba(int width, int height, const uint8_t *pixels, size_t length);
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
/* Monotonic elapsed clock in seconds with an unspecified origin. Available
 * without a window. Return 0 on clock failure. For an explicit failure result,
 * use mv_monotonic_time. */
double mv_platform_time(void);
/* Nonpositive, nonfinite, or unrepresentable durations are ignored.
 * Durations must be below UINT32_MAX/1000 seconds on Windows and INT_MAX
 * seconds on other platforms. The wait can end early if interrupted. */
void mv_platform_wait(double seconds);

/* Write monotonic elapsed-time clock seconds with an unspecified origin.
 * Return 1 on success, or 0 without changing the output on failure. */
int mv_monotonic_time(double *seconds);

#ifdef __cplusplus
}
#endif
#endif
