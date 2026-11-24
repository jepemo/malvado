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
static int inputs, commands, last_command;
static int focused;
int
mv_android_activity_has_focus(struct android_app *a)
{
    assert(a == &app);
    return focused;
}
static int text_samples, text_queries;
uint32_t mv_android_key_codepoint(struct android_app *a, int device, int key, int meta)
{
    assert(a == &app && device == -1 && meta == 0);
    text_queries++;
    return key == AKEYCODE_A ? 'a' : 0;
}
void mv_text_input_sample(uint32_t codepoint)
{
    assert(codepoint == 'a');
    text_samples++;
}
static int available = 1;
static int script[8], script_count, script_index, processed;
struct android_app *
GetAndroidApp(void)
{
    return available ? &app : NULL;
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
    last_command = code;
    commands++;
}
static int32_t
event(int code, int action)
{
    AInputEvent e = {AINPUT_EVENT_TYPE_KEY, code, action, 0, 0};
    return app.onInputEvent(&app, &e);
}
static void
motion(int action, int source, int buttons)
{
    AInputEvent e = {AINPUT_EVENT_TYPE_MOTION, 0, action, source, buttons};
    assert(app.onInputEvent(&app, &e) == 7);
}

static void
check_pointer(void)
{
    app.onAppCmd(&app, APP_CMD_GAINED_FOCUS);
    motion(AMOTION_EVENT_ACTION_DOWN, AINPUT_SOURCE_CLASS_POINTER, 0);
    motion(AMOTION_EVENT_ACTION_UP, AINPUT_SOURCE_CLASS_POINTER, 0);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT) && !mv_mouse_pressed(MV_MOUSE_LEFT));
    mv_input_tick();
    assert(mv_mouse_pressed(MV_MOUSE_LEFT) && mv_mouse_released(MV_MOUSE_LEFT));
    mv_input_tick();
    assert(!mv_mouse_pressed(MV_MOUSE_LEFT) && !mv_mouse_released(MV_MOUSE_LEFT));

    motion(AMOTION_EVENT_ACTION_DOWN, AINPUT_SOURCE_CLASS_POINTER, 0);
    assert(mv_android_mouse_down(MV_MOUSE_LEFT));
    mv_input_tick();
    assert(mv_mouse_pressed(MV_MOUSE_LEFT) && !mv_mouse_released(MV_MOUSE_LEFT));
    app.onAppCmd(&app, APP_CMD_PAUSE);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
    motion(AMOTION_EVENT_ACTION_DOWN, AINPUT_SOURCE_CLASS_POINTER, 0);
    mv_input_tick();
    assert(!mv_mouse_pressed(MV_MOUSE_LEFT) && mv_mouse_released(MV_MOUSE_LEFT));
    app.onAppCmd(&app, APP_CMD_RESUME);
    motion(2, AINPUT_SOURCE_CLASS_POINTER, 1);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
    motion(AMOTION_EVENT_ACTION_DOWN, AINPUT_SOURCE_CLASS_POINTER, 0);
    assert(mv_android_mouse_down(MV_MOUSE_LEFT));
    motion(AMOTION_EVENT_ACTION_CANCEL, AINPUT_SOURCE_CLASS_POINTER, 0);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
    mv_input_tick();
    assert(mv_mouse_pressed(MV_MOUSE_LEFT) && mv_mouse_released(MV_MOUSE_LEFT));
    motion(AMOTION_EVENT_ACTION_DOWN, AINPUT_SOURCE_CLASS_POINTER, 2 | 4);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
    assert(mv_android_mouse_down(MV_MOUSE_MIDDLE) && mv_android_mouse_down(MV_MOUSE_RIGHT));
    motion(AMOTION_EVENT_ACTION_BUTTON_RELEASE, AINPUT_SOURCE_CLASS_POINTER, 4);
    assert(mv_android_mouse_down(MV_MOUSE_MIDDLE) && !mv_android_mouse_down(MV_MOUSE_RIGHT));
    motion(AMOTION_EVENT_ACTION_UP, AINPUT_SOURCE_CLASS_POINTER, 0);
    assert(!mv_android_mouse_down(MV_MOUSE_MIDDLE));
    motion(AMOTION_EVENT_ACTION_DOWN, 0, 1);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
    assert(!mv_android_mouse_down((MvMouseButton)99));
    motion(AMOTION_EVENT_ACTION_DOWN, AINPUT_SOURCE_CLASS_POINTER, 1);
    app.onAppCmd(&app, APP_CMD_LOST_FOCUS);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
    motion(AMOTION_EVENT_ACTION_DOWN, AINPUT_SOURCE_CLASS_POINTER, 1);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
    mv_input_tick();
    assert(mv_mouse_pressed(MV_MOUSE_LEFT) && mv_mouse_released(MV_MOUSE_LEFT));
    app.onAppCmd(&app, APP_CMD_GAINED_FOCUS);
    motion(2, AINPUT_SOURCE_CLASS_POINTER, 1);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
    motion(AMOTION_EVENT_ACTION_DOWN, AINPUT_SOURCE_CLASS_POINTER, 1);
    app.onAppCmd(&app, APP_CMD_TERM_WINDOW);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
    app.onAppCmd(&app, APP_CMD_INIT_WINDOW);
    motion(2, AINPUT_SOURCE_CLASS_POINTER, 1);
    assert(!mv_android_mouse_down(MV_MOUSE_LEFT));
}

static void
poll_command(struct android_app *a, struct android_poll_source *source)
{
    (void)source;
    processed++;
    if (script[script_index - 1] == APP_CMD_DESTROY) a->destroyRequested = 1;
    if (a->onAppCmd) a->onAppCmd(a, script[script_index - 1]);
}

int
ALooper_pollOnce(int timeout, int *fd, int *events, void **data)
{
    (void)fd;
    (void)events;
    assert(timeout == -1);
    static struct android_poll_source source = {poll_command};
    if (script_index == script_count) return ALOOPER_POLL_ERROR;
    int cmd = script[script_index++];
    if (cmd == APP_CMD_INIT_WINDOW) app.pendingWindow = (void *)1;
    *data = cmd >= 0 ? &source : NULL;
    return cmd >= 0 ? 1 : cmd;
}

static void
check_startup(void)
{
    app.onAppCmd = command;
    available = 0;
    assert(!mv_android_window_wait_surface());
    available = 1;
    app.destroyRequested = 1;
    app.pendingWindow = (void *)1;
    assert(!mv_android_window_wait_surface());
    app.destroyRequested = 0;
    app.pendingWindow = NULL;
    script[0] = APP_CMD_PAUSE;
    script[1] = APP_CMD_DESTROY;
    script_count = 2;
    script_index = processed = 0;
    assert(!mv_android_window_wait_surface());
    assert(processed == 2 && commands == 2 && app.destroyRequested);
    app.destroyRequested = 0;
    script[0] = -1; /* A looper wake is not an initialization failure. */
    script[1] = APP_CMD_PAUSE;
    script[2] = APP_CMD_INIT_WINDOW;
    script_count = 3;
    script_index = processed = 0;
    assert(mv_android_window_wait_surface());
    assert(processed == 1 && app.window == NULL && app.pendingWindow);
    assert(script_index == 3); /* INIT remains unprocessed for raylib. */
    assert(mv_android_window_wait_surface());
    assert(script_index == 3);
    app.pendingWindow = NULL;
    app.window = (void *)1;
    assert(mv_android_window_wait_surface());
    app.window = NULL;
    script_count = script_index = 0;
    assert(!mv_android_window_wait_surface());
    assert(commands == 3 && app.onAppCmd == command);
    commands = 0;
    app.onAppCmd = NULL;
}

int
main(void)
{
    assert(pthread_mutex_init(&app.mutex, NULL) == 0);
    check_startup();
    app.onInputEvent = input;
    app.onAppCmd = command;
    for (int reopen = 0; reopen < 2; ++reopen) {
        mv_input_open();
        mv_android_window_attach();
        check_pointer();
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
        int before_text = text_samples;
        int before_queries = text_queries;
        event(AKEYCODE_A, AKEY_EVENT_ACTION_DOWN);
        event(AKEYCODE_A, AKEY_EVENT_ACTION_DOWN);
        assert(text_samples == before_text + 2 && text_queries == before_queries + 2);
        event(AKEYCODE_A, AKEY_EVENT_ACTION_UP);
        assert(text_samples == before_text + 2 && text_queries == before_queries + 2);
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
        mv_input_reset();
        assert(app.onInputEvent == input && app.onAppCmd == command);
        assert(!mv_android_key_down(MV_KEY_A) && !mv_android_window_close_requested());
    }
    assert(inputs && commands && text_queries && text_samples);
    int before = commands;
    focused = 1;
    mv_android_window_attach();
    assert(commands == before + 1 && last_command == APP_CMD_GAINED_FOCUS);
    assert(!mv_android_key_down(MV_KEY_A) && !mv_android_mouse_down(MV_MOUSE_LEFT));
    mv_android_window_detach();
    assert(app.onAppCmd == command && app.onInputEvent == input);
    focused = 0;
    mv_android_window_attach();
    assert(commands == before + 1);
    mv_android_window_detach();
    assert(pthread_mutex_destroy(&app.mutex) == 0);
}
