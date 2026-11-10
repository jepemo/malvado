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
    return 0;
}
