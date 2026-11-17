/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */
#include "android_window.h"
#include "input_internal.h"
#include <android_native_app_glue.h>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
static struct android_app app;
static int inputs, commands;
struct android_app *
GetAndroidApp(void)
{
    return &app;
}
static int32_t
input(struct android_app *a, AInputEvent *e)
{
    (void)a;
    (void)e;
    inputs++;
    return 7;
}
static void
command(struct android_app *a, int32_t code)
{
    (void)a;
    (void)code;
    commands++;
}
static int32_t
event(int code, int action)
{
    AInputEvent e = {AINPUT_EVENT_TYPE_KEY, code, action};
    return app.onInputEvent(&app, &e);
}
int
main(void)
{
    app.onInputEvent = input;
    app.onAppCmd = command;
    for (int reopen = 0; reopen < 2; ++reopen) {
        mv_keyboard_open();
        mv_android_window_attach();
        const int native[] = {AKEYCODE_A, AKEYCODE_DPAD_LEFT, AKEYCODE_0, AKEYCODE_Z, AKEYCODE_DEL};
        const MvKey keys[] = {MV_KEY_A, MV_KEY_LEFT, MV_KEY_0, MV_KEY_Z, MV_KEY_BACKSPACE};
        for (unsigned i = 0; i < sizeof(keys)/sizeof(keys[0]); ++i) {
            event(native[i], AKEY_EVENT_ACTION_DOWN);
            mv_input_tick();
            assert(mv_android_key_down(keys[i]) && mv_key_pressed(keys[i]));
            event(native[i], AKEY_EVENT_ACTION_DOWN);
            mv_input_tick();
            assert(!mv_key_pressed(keys[i]));
            app.onAppCmd(&app, APP_CMD_PAUSE);
            mv_input_tick();
            assert(!mv_android_key_down(keys[i]) && mv_key_released(keys[i]));
            assert(mv_key_released(keys[i]));
            mv_input_tick();
            assert(!mv_key_released(keys[i]));
            /* A missed background key-up must not block a fresh press. */
            event(native[i], AKEY_EVENT_ACTION_DOWN);
            mv_input_tick();
            assert(mv_key_pressed(keys[i]) && mv_android_key_down(keys[i]));
            event(native[i], AKEY_EVENT_ACTION_UP);
            mv_input_tick();
            assert(mv_key_released(keys[i]) && !mv_android_key_down(keys[i]));
        }
        event(AKEYCODE_A, 0);
        event(AKEYCODE_A, 1);
        mv_input_tick();
        assert(mv_key_pressed(MV_KEY_A) && mv_key_released(MV_KEY_A));
        assert(event(-99, 0) == 7);
        assert(!mv_android_key_down((MvKey)-1) && !mv_android_key_down((MvKey)999));
        event(AKEYCODE_A, 0);
        app.onAppCmd(&app, APP_CMD_LOST_FOCUS);
        assert(!mv_android_key_down(MV_KEY_A));
        event(AKEYCODE_A, 0);
        app.onAppCmd(&app, APP_CMD_TERM_WINDOW);
        assert(!mv_android_key_down(MV_KEY_A));
        assert(event(AKEYCODE_BACK, 0) == 1);
        assert(mv_android_window_close_requested());
        mv_android_window_detach();
        mv_keyboard_reset();
        assert(app.onInputEvent == input && app.onAppCmd == command);
        assert(!mv_android_key_down(MV_KEY_A) && !mv_android_window_close_requested());
    }
    assert(inputs && commands);
}
