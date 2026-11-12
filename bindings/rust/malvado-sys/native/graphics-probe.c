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
#include <stdio.h>
_Static_assert(_Generic(&mv_window_open, int (*)(int, int, const char *): 1, default: 0), "mv_window_open signature");
_Static_assert(_Generic(&mv_window_set_title, int (*)(const char *): 1, default: 0), "mv_window_set_title signature");
_Static_assert(_Generic(&mv_window_size, int (*)(int *, int *): 1, default: 0), "mv_window_size signature");
_Static_assert(_Generic(&mv_window_close, void (*)(void): 1, default: 0), "mv_window_close signature");
_Static_assert(_Generic(&mv_window_should_close, int (*)(void): 1, default: 0), "mv_window_should_close signature");
_Static_assert(_Generic(&mv_window_begin, void (*)(MvColor): 1, default: 0), "mv_window_begin signature");
_Static_assert(_Generic(&mv_window_end, void (*)(void): 1, default: 0), "mv_window_end signature");
_Static_assert(_Generic(&mv_input_tick, void (*)(void): 1, default: 0), "mv_input_tick signature");
_Static_assert(_Generic(&mv_key_down, int (*)(MvKey): 1, default: 0), "mv_key_down signature");
_Static_assert(_Generic(&mv_key_pressed, int (*)(MvKey): 1, default: 0), "mv_key_pressed signature");
_Static_assert(_Generic(&mv_key_released, int (*)(MvKey): 1, default: 0), "mv_key_released signature");
_Static_assert(_Generic(&mv_window_clip_begin, int (*)(int, int, int, int): 1, default: 0), "mv_window_clip_begin signature");
_Static_assert(_Generic(&mv_window_clip_end, int (*)(void): 1, default: 0), "mv_window_clip_end signature");
_Static_assert(_Generic(&mv_window_scroll_begin, int (*)(float, float): 1, default: 0), "mv_window_scroll_begin signature");
_Static_assert(_Generic(&mv_window_scroll_end, int (*)(void): 1, default: 0), "mv_window_scroll_end signature");
_Static_assert(_Generic(&mv_window_draw_text, void (*)(const char *, int, int, int, MvColor): 1, default: 0), "mv_window_draw_text signature");
_Static_assert(_Generic(&mv_window_text_width, int (*)(const char *, int): 1, default: 0), "mv_window_text_width signature");
_Static_assert(_Generic(&mv_window_draw_box, int (*)(int, int, int, int, int, MvColor): 1, default: 0), "mv_window_draw_box signature");
_Static_assert(_Generic(&mv_window_draw_circle, int (*)(int, int, int, int, MvColor): 1, default: 0), "mv_window_draw_circle signature");
_Static_assert(_Generic(&mv_window_draw_transformed_rect, int (*)(float, float, float, float, float, float, float, MvColor): 1, default: 0), "mv_window_draw_transformed_rect signature");
_Static_assert(_Generic(&mv_window_draw_filled_circle, int (*)(float, float, float, MvColor): 1, default: 0), "mv_window_draw_filled_circle signature");
_Static_assert(_Generic(&mv_image_load, MvImage * (*)(const char *): 1, default: 0), "mv_image_load signature");
_Static_assert(_Generic(&mv_image_from_rgba, MvImage * (*)(int, int, const uint8_t *, size_t): 1, default: 0), "mv_image_from_rgba signature");
_Static_assert(_Generic(&mv_image_destroy, void (*)(MvImage *): 1, default: 0), "mv_image_destroy signature");
_Static_assert(_Generic(&mv_image_width, int (*)(const MvImage *): 1, default: 0), "mv_image_width signature");
_Static_assert(_Generic(&mv_image_height, int (*)(const MvImage *): 1, default: 0), "mv_image_height signature");
_Static_assert(_Generic(&mv_image_draw, void (*)(const MvImage *, int, int): 1, default: 0), "mv_image_draw signature");
_Static_assert(_Generic(&mv_image_region_valid, int (*)(const MvImage *, MvImageRegion, MvImageTransform): 1, default: 0), "mv_image_region_valid signature");
_Static_assert(_Generic(&mv_image_draw_region, int (*)(const MvImage *, MvImageRegion, MvImageTransform): 1, default: 0), "mv_image_draw_region signature");
_Static_assert(_Generic(&mv_font_load, MvFont * (*)(const char *, int): 1, default: 0), "mv_font_load signature");
_Static_assert(_Generic(&mv_font_load_codepoints, MvFont * (*)(const char *, int, const uint32_t *, size_t): 1, default: 0), "mv_font_load_codepoints signature");
_Static_assert(_Generic(&mv_font_destroy, void (*)(MvFont *): 1, default: 0), "mv_font_destroy signature");
_Static_assert(_Generic(&mv_font_text_width, int (*)(const MvFont *, const char *, int): 1, default: 0), "mv_font_text_width signature");
_Static_assert(_Generic(&mv_font_draw, int (*)(const MvFont *, const char *, int, int, int, MvColor): 1, default: 0), "mv_font_draw signature");
_Static_assert(_Generic(&mv_mouse_position, int (*)(int *, int *): 1, default: 0), "mv_mouse_position signature");
_Static_assert(_Generic(&mv_mouse_down, int (*)(MvMouseButton): 1, default: 0), "mv_mouse_down signature");
_Static_assert(_Generic(&mv_pad_connected, int (*)(int): 1, default: 0), "mv_pad_connected signature");
_Static_assert(_Generic(&mv_pad_button_down, int (*)(int, MvPadButton): 1, default: 0), "mv_pad_button_down signature");
_Static_assert(_Generic(&mv_pad_axis, float (*)(int, MvPadAxis): 1, default: 0), "mv_pad_axis signature");
_Static_assert(_Generic(&mv_text_input_next, int (*)(uint32_t *): 1, default: 0), "mv_text_input_next signature");
_Static_assert(_Generic(&mv_audio_open, int (*)(void): 1, default: 0), "mv_audio_open signature");
_Static_assert(_Generic(&mv_audio_close, void (*)(void): 1, default: 0), "mv_audio_close signature");
_Static_assert(_Generic(&mv_sound_load, MvSound * (*)(const char *): 1, default: 0), "mv_sound_load signature");
_Static_assert(_Generic(&mv_sound_clone, MvSound * (*)(const MvSound *): 1, default: 0), "mv_sound_clone signature");
_Static_assert(_Generic(&mv_sound_destroy, void (*)(MvSound *): 1, default: 0), "mv_sound_destroy signature");
_Static_assert(_Generic(&mv_sound_play, int (*)(MvSound *): 1, default: 0), "mv_sound_play signature");
_Static_assert(_Generic(&mv_sound_stop, int (*)(MvSound *): 1, default: 0), "mv_sound_stop signature");
_Static_assert(_Generic(&mv_sound_pause, int (*)(MvSound *): 1, default: 0), "mv_sound_pause signature");
_Static_assert(_Generic(&mv_sound_resume, int (*)(MvSound *): 1, default: 0), "mv_sound_resume signature");
_Static_assert(_Generic(&mv_sound_playing, int (*)(const MvSound *): 1, default: 0), "mv_sound_playing signature");
_Static_assert(_Generic(&mv_sound_set_volume, int (*)(MvSound *, float): 1, default: 0), "mv_sound_set_volume signature");
_Static_assert(_Generic(&mv_music_load, MvMusic * (*)(const char *, int): 1, default: 0), "mv_music_load signature");
_Static_assert(_Generic(&mv_music_destroy, void (*)(MvMusic *): 1, default: 0), "mv_music_destroy signature");
_Static_assert(_Generic(&mv_music_play, int (*)(MvMusic *): 1, default: 0), "mv_music_play signature");
_Static_assert(_Generic(&mv_music_update, int (*)(MvMusic *): 1, default: 0), "mv_music_update signature");
_Static_assert(_Generic(&mv_music_stop, int (*)(MvMusic *): 1, default: 0), "mv_music_stop signature");
_Static_assert(_Generic(&mv_music_pause, int (*)(MvMusic *): 1, default: 0), "mv_music_pause signature");
_Static_assert(_Generic(&mv_music_resume, int (*)(MvMusic *): 1, default: 0), "mv_music_resume signature");
_Static_assert(_Generic(&mv_music_playing, int (*)(const MvMusic *): 1, default: 0), "mv_music_playing signature");
_Static_assert(_Generic(&mv_music_set_volume, int (*)(MvMusic *, float): 1, default: 0), "mv_music_set_volume signature");
_Static_assert(_Generic(&mv_fade_queue_push, int (*)(MvFadeQueue *, MvColor, unsigned): 1, default: 0), "mv_fade_queue_push signature");
_Static_assert(_Generic(&mv_fade_queue_step, int (*)(MvFadeQueue *): 1, default: 0), "mv_fade_queue_step signature");
_Static_assert(_Generic(&mv_fade_queue_active, int (*)(const MvFadeQueue *): 1, default: 0), "mv_fade_queue_active signature");
_Static_assert(_Generic(&mv_window_draw_fade, int (*)(MvColor): 1, default: 0), "mv_window_draw_fade signature");
_Static_assert(_Generic(&mv_window_draw_process_graphics, int (*)(MvContext *): 1, default: 0), "mv_window_draw_process_graphics signature");
_Static_assert(_Generic(&mv_window_draw_graphic, int (*)(MvCollisionShape, MvCollisionTransform, MvColor): 1, default: 0), "mv_window_draw_graphic signature");
_Static_assert(_Generic(&mv_draw_state_init, int (*)(MvDrawState *): 1, default: 0), "mv_draw_state_init signature");
_Static_assert(_Generic(&mv_draw_state_color, int (*)(MvDrawState *, int64_t, int64_t, int64_t, int64_t): 1, default: 0), "mv_draw_state_color signature");
_Static_assert(_Generic(&mv_draw_state_background, int (*)(MvDrawState *, int64_t, int64_t, int64_t): 1, default: 0), "mv_draw_state_background signature");
int main(void)
{
    printf("%zu %zu ", sizeof(MvColor), _Alignof(MvColor));
    printf("%zu ", offsetof(MvColor, r));
    printf("%zu ", offsetof(MvColor, g));
    printf("%zu ", offsetof(MvColor, b));
    printf("%zu ", offsetof(MvColor, a));
    printf("%zu %zu ", sizeof(MvKey), _Alignof(MvKey));
    printf("%u ", (unsigned)MV_KEY_0);
    printf("%u ", (unsigned)MV_KEY_1);
    printf("%u ", (unsigned)MV_KEY_2);
    printf("%u ", (unsigned)MV_KEY_3);
    printf("%u ", (unsigned)MV_KEY_4);
    printf("%u ", (unsigned)MV_KEY_5);
    printf("%u ", (unsigned)MV_KEY_6);
    printf("%u ", (unsigned)MV_KEY_7);
    printf("%u ", (unsigned)MV_KEY_8);
    printf("%u ", (unsigned)MV_KEY_9);
    printf("%u ", (unsigned)MV_KEY_A);
    printf("%u ", (unsigned)MV_KEY_B);
    printf("%u ", (unsigned)MV_KEY_C);
    printf("%u ", (unsigned)MV_KEY_D);
    printf("%u ", (unsigned)MV_KEY_E);
    printf("%u ", (unsigned)MV_KEY_F);
    printf("%u ", (unsigned)MV_KEY_G);
    printf("%u ", (unsigned)MV_KEY_H);
    printf("%u ", (unsigned)MV_KEY_I);
    printf("%u ", (unsigned)MV_KEY_J);
    printf("%u ", (unsigned)MV_KEY_K);
    printf("%u ", (unsigned)MV_KEY_L);
    printf("%u ", (unsigned)MV_KEY_M);
    printf("%u ", (unsigned)MV_KEY_N);
    printf("%u ", (unsigned)MV_KEY_O);
    printf("%u ", (unsigned)MV_KEY_P);
    printf("%u ", (unsigned)MV_KEY_Q);
    printf("%u ", (unsigned)MV_KEY_R);
    printf("%u ", (unsigned)MV_KEY_S);
    printf("%u ", (unsigned)MV_KEY_T);
    printf("%u ", (unsigned)MV_KEY_U);
    printf("%u ", (unsigned)MV_KEY_V);
    printf("%u ", (unsigned)MV_KEY_W);
    printf("%u ", (unsigned)MV_KEY_X);
    printf("%u ", (unsigned)MV_KEY_Y);
    printf("%u ", (unsigned)MV_KEY_Z);
    printf("%u ", (unsigned)MV_KEY_ESCAPE);
    printf("%u ", (unsigned)MV_KEY_SPACE);
    printf("%u ", (unsigned)MV_KEY_LEFT);
    printf("%u ", (unsigned)MV_KEY_RIGHT);
    printf("%u ", (unsigned)MV_KEY_UP);
    printf("%u ", (unsigned)MV_KEY_DOWN);
    printf("%u ", (unsigned)MV_KEY_ENTER);
    printf("%u ", (unsigned)MV_KEY_TAB);
    printf("%u ", (unsigned)MV_KEY_BACKSPACE);
    printf("%zu %zu ", sizeof(MvImageRegion), _Alignof(MvImageRegion));
    printf("%zu ", offsetof(MvImageRegion, x));
    printf("%zu ", offsetof(MvImageRegion, y));
    printf("%zu ", offsetof(MvImageRegion, width));
    printf("%zu ", offsetof(MvImageRegion, height));
    printf("%zu %zu ", sizeof(MvImageTransform), _Alignof(MvImageTransform));
    printf("%zu ", offsetof(MvImageTransform, x));
    printf("%zu ", offsetof(MvImageTransform, y));
    printf("%zu ", offsetof(MvImageTransform, pivot_x));
    printf("%zu ", offsetof(MvImageTransform, pivot_y));
    printf("%zu ", offsetof(MvImageTransform, scale));
    printf("%zu ", offsetof(MvImageTransform, rotation));
    printf("%zu %zu ", sizeof(MvMouseButton), _Alignof(MvMouseButton));
    printf("%u ", (unsigned)MV_MOUSE_LEFT);
    printf("%u ", (unsigned)MV_MOUSE_MIDDLE);
    printf("%u ", (unsigned)MV_MOUSE_RIGHT);
    printf("%zu %zu ", sizeof(MvPadButton), _Alignof(MvPadButton));
    printf("%u ", (unsigned)MV_PAD_A);
    printf("%u ", (unsigned)MV_PAD_B);
    printf("%u ", (unsigned)MV_PAD_X);
    printf("%u ", (unsigned)MV_PAD_Y);
    printf("%u ", (unsigned)MV_PAD_UP);
    printf("%u ", (unsigned)MV_PAD_DOWN);
    printf("%u ", (unsigned)MV_PAD_LEFT);
    printf("%u ", (unsigned)MV_PAD_RIGHT);
    printf("%u ", (unsigned)MV_PAD_LB);
    printf("%u ", (unsigned)MV_PAD_RB);
    printf("%u ", (unsigned)MV_PAD_BACK);
    printf("%u ", (unsigned)MV_PAD_START);
    printf("%zu %zu ", sizeof(MvPadAxis), _Alignof(MvPadAxis));
    printf("%u ", (unsigned)MV_PAD_LEFT_X);
    printf("%u ", (unsigned)MV_PAD_LEFT_Y);
    printf("%u ", (unsigned)MV_PAD_RIGHT_X);
    printf("%u ", (unsigned)MV_PAD_RIGHT_Y);
    printf("%u ", (unsigned)MV_PAD_LEFT_TRIGGER);
    printf("%u ", (unsigned)MV_PAD_RIGHT_TRIGGER);
    printf("%zu %zu ", sizeof(MvFade), _Alignof(MvFade));
    printf("%zu ", offsetof(MvFade, current));
    printf("%zu ", offsetof(MvFade, start));
    printf("%zu ", offsetof(MvFade, target));
    printf("%zu ", offsetof(MvFade, duration));
    printf("%zu ", offsetof(MvFade, elapsed));
    printf("%zu %zu ", sizeof(MvFadeTransition), _Alignof(MvFadeTransition));
    printf("%zu ", offsetof(MvFadeTransition, target));
    printf("%zu ", offsetof(MvFadeTransition, ticks));
    printf("%zu %zu ", sizeof(MvFadeQueue), _Alignof(MvFadeQueue));
    printf("%zu ", offsetof(MvFadeQueue, fade));
    printf("%zu ", offsetof(MvFadeQueue, pending));
    printf("%zu ", offsetof(MvFadeQueue, head));
    printf("%zu ", offsetof(MvFadeQueue, count));
    printf("%zu %zu ", sizeof(MvDrawState), _Alignof(MvDrawState));
    printf("%zu ", offsetof(MvDrawState, color));
    printf("%zu ", offsetof(MvDrawState, background));
    printf("%zu ", sizeof(MvImageGraphic));
    printf("%zu ", _Alignof(MvImageGraphic));
    printf("%zu ", offsetof(MvImageGraphic, image));
    printf("%zu ", offsetof(MvImageGraphic, region));
    printf("%zu ", offsetof(MvImageGraphic, pivot_x));
    printf("%zu ", offsetof(MvImageGraphic, pivot_y));
    printf("%zu ", sizeof(MvImageAnimationFrame));
    printf("%zu ", _Alignof(MvImageAnimationFrame));
    printf("%zu ", offsetof(MvImageAnimationFrame, graphic));
    printf("%zu ", offsetof(MvImageAnimationFrame, duration_us));
    return 0;
}

_Static_assert(_Generic(&mv_resource_adopt_image, MvResult (*)(MvContext *, const MvProcessRef *, MvImage *, MvResource **): 1, default: 0), "mv_resource_adopt_image signature");

_Static_assert(_Generic(&mv_resource_adopt_font, MvResult (*)(MvContext *, const MvProcessRef *, MvFont *, MvResource **): 1, default: 0), "mv_resource_adopt_font signature");

_Static_assert(_Generic(&mv_resource_adopt_sound, MvResult (*)(MvContext *, const MvProcessRef *, MvSound *, MvResource **): 1, default: 0), "mv_resource_adopt_sound signature");

_Static_assert(_Generic(&mv_resource_adopt_music, MvResult (*)(MvContext *, const MvProcessRef *, MvMusic *, MvResource **): 1, default: 0), "mv_resource_adopt_music signature");

_Static_assert(_Generic(&mv_image_from_png, MvImage * (*)(const uint8_t *, size_t): 1, default: 0), "mv_image_from_png signature");

_Static_assert(_Generic(&mv_image_retain, int (*)(MvImage *): 1, default: 0), "mv_image_retain signature");

_Static_assert(_Generic(&mv_process_image_set, MvResult (*)(MvContext *, MvProcessRef, MvImageGraphic): 1, default: 0), "mv_process_image_set signature");
_Static_assert(_Generic(&mv_process_image_get, MvResult (*)(MvContext *, MvProcessRef, MvImageGraphic *, int *): 1, default: 0), "mv_process_image_get signature");

_Static_assert(_Generic(&mv_process_image_set_owned, MvResult (*)(MvContext *, MvProcessRef, MvImageGraphic, void *, MvResourceDestroy): 1, default: 0), "mv_process_image_set_owned signature");

_Static_assert(_Generic(&mv_process_image_animation_set, MvResult (*)(MvContext *, MvProcessRef, const MvImageAnimationFrame *, size_t, int, unsigned): 1, default: 0), "mv_process_image_animation_set signature");
_Static_assert(_Generic(&mv_process_image_animation_set_owned, MvResult (*)(MvContext *, MvProcessRef, const MvImageAnimationFrame *, size_t, int, unsigned, void *, MvResourceDestroy): 1, default: 0), "mv_process_image_animation_set_owned signature");

_Static_assert(_Generic(&mv_process_image_animation_bind_owned, MvResult (*)(MvContext *, MvProcessRef, MvAnimation *, const MvImageGraphic *, size_t, unsigned, void *, MvResourceDestroy): 1, default: 0), "mv_process_image_animation_bind_owned signature");

_Static_assert(_Generic(&mv_window_draw_process_commands, int (*)(MvContext *, MvProcessRef): 1, default: 0), "mv_window_draw_process_commands signature");
_Static_assert(_Generic(&mv_process_draw_image_region_owned, MvResult (*)(MvContext *, MvProcessRef, MvImage *, MvImageRegion, MvImageTransform, void *, MvResourceDestroy): 1, default: 0), "mv_process_draw_image_region_owned signature");
_Static_assert(_Generic(&mv_process_draw_image_region, MvResult (*)(MvContext *, MvProcessRef, MvImage *, MvImageRegion, MvImageTransform): 1, default: 0), "mv_process_draw_image_region signature");

_Static_assert(_Generic(&mv_font_retain, int (*)(MvFont *): 1, default: 0), "mv_font_retain signature");

_Static_assert(_Generic(&mv_text_entry_font, MvResult (*)(MvContext *, uint64_t, MvFont *, int, MvColor): 1, default: 0), "mv_text_entry_font signature");
_Static_assert(_Generic(&mv_text_entry_font_clear, MvResult (*)(MvContext *, uint64_t): 1, default: 0), "mv_text_entry_font_clear signature");
_Static_assert(_Generic(&mv_window_draw_texts, int (*)(MvContext *): 1, default: 0), "mv_window_draw_texts signature");

_Static_assert(_Generic(&mv_text_entry_font_owned, MvResult (*)(MvContext *, uint64_t, MvFont *, int, MvColor, void *, MvResourceDestroy): 1, default: 0), "mv_text_entry_font_owned signature");
