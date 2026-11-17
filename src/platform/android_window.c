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
#include <android_native_app_glue.h>
#include <string.h>
#include "input_internal.h"

extern struct android_app *GetAndroidApp(void);
static struct android_app *window_app;
static int32_t (*previous_input)(struct android_app *, AInputEvent *);
static int close_requested;
static void (*previous_command)(struct android_app *, int32_t);
static unsigned char held[MV_KEY_BACKSPACE + 1];
static int pointer_buttons;

static int
key_from_android(int code)
{
    if (code >= AKEYCODE_A && code <= AKEYCODE_Z) return MV_KEY_A + code - AKEYCODE_A;
    if (code >= AKEYCODE_0 && code <= AKEYCODE_9) return MV_KEY_0 + code - AKEYCODE_0;
    switch (code) {
    case AKEYCODE_ESCAPE: return MV_KEY_ESCAPE;
    case AKEYCODE_SPACE: return MV_KEY_SPACE;
    case AKEYCODE_DPAD_LEFT: return MV_KEY_LEFT;
    case AKEYCODE_DPAD_RIGHT: return MV_KEY_RIGHT;
    case AKEYCODE_DPAD_UP: return MV_KEY_UP;
    case AKEYCODE_DPAD_DOWN: return MV_KEY_DOWN;
    case AKEYCODE_ENTER: return MV_KEY_ENTER;
    case AKEYCODE_TAB: return MV_KEY_TAB;
    case AKEYCODE_DEL: return MV_KEY_BACKSPACE;
    default: return -1;
    }
}

static void
window_command(struct android_app *app, int32_t command)
{
    if (previous_command) previous_command(app, command);
    if (command == APP_CMD_PAUSE || command == APP_CMD_LOST_FOCUS || command == APP_CMD_TERM_WINDOW) {
        /* Android may deliver releases to another Activity. Queue key edges
         * and neutralize pointer buttons; foreground needs fresh presses. */
        for (int key = 0; key <= MV_KEY_BACKSPACE; ++key) {
            if (held[key]) mv_keyboard_sample_held((MvKey)key, 0);
        }
        memset(held, 0, sizeof(held));
        pointer_buttons = 0;
    }
}

static int32_t
window_input(struct android_app *app, AInputEvent *event)
{
    int32_t handled = previous_input ? previous_input(app, event) : 0;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        int key = key_from_android(AKeyEvent_getKeyCode(event));
        int action = AKeyEvent_getAction(event);
        if (key >= 0 && (action == AKEY_EVENT_ACTION_DOWN || action == AKEY_EVENT_ACTION_UP)) {
            held[key] = action == AKEY_EVENT_ACTION_DOWN;
            mv_keyboard_sample_held((MvKey)key, held[key]);
        }
    }
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION &&
        (AInputEvent_getSource(event) & AINPUT_SOURCE_CLASS_POINTER)) {
        int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        if (action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_BUTTON_PRESS) {
            pointer_buttons = AMotionEvent_getButtonState(event);
            if (!pointer_buttons) pointer_buttons = AMOTION_EVENT_BUTTON_PRIMARY;
        } else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_CANCEL) {
            pointer_buttons = 0;
        } else if (action == AMOTION_EVENT_ACTION_BUTTON_RELEASE) {
            pointer_buttons = AMotionEvent_getButtonState(event);
        }
        /* MOVE cannot revive a contact cancelled by focus loss. */
    }
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY &&
        AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
        if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
            close_requested = 1;
        return 1;
    }
    return handled;
}

/* Wait without allocating graphics resources. The Java window callback sets
 * pendingWindow under this mutex before writing INIT_WINDOW to the command
 * pipe. Leave that command unread for raylib's own initialization callback. */
int
mv_android_window_wait_surface(void)
{
    struct android_app *app = GetAndroidApp();
    if (!app) return 0;
    for (;;) {
        if (app->destroyRequested) return 0;
        pthread_mutex_lock(&app->mutex);
        int ready = app->window != NULL || app->pendingWindow != NULL;
        pthread_mutex_unlock(&app->mutex);
        if (ready) return 1;
        struct android_poll_source *source = NULL;
        int events;
        int result = ALooper_pollOnce(-1, NULL, &events, (void **)&source);
        if (result == ALOOPER_POLL_ERROR) return 0;
        /* A window may have arrived while poll was blocked. Do not consume
         * its command, even if earlier non-window commands are also queued. */
        if (app->destroyRequested) return 0;
        pthread_mutex_lock(&app->mutex);
        ready = app->pendingWindow != NULL;
        pthread_mutex_unlock(&app->mutex);
        if (!ready && result >= 0 && source && source->process)
            source->process(app, source);
    }
}

void
mv_android_window_attach(void)
{
    window_app = GetAndroidApp();
    close_requested = 0;
    pointer_buttons = 0;
    memset(held, 0, sizeof(held));
    if (!window_app) return;
    previous_input = window_app->onInputEvent;
    window_app->onInputEvent = window_input;
    previous_command = window_app->onAppCmd;
    window_app->onAppCmd = window_command;
    /* A recreated Activity can retain its focused Window without another
     * GAINED_FOCUS event. Initialize raylib from the actual Java focus state. */
    if (previous_command && mv_android_activity_has_focus(window_app))
        previous_command(window_app, APP_CMD_GAINED_FOCUS);
}

void
mv_android_window_detach(void)
{
    if (window_app && window_app->onInputEvent == window_input)
        window_app->onInputEvent = previous_input;
    if (window_app && window_app->onAppCmd == window_command)
        window_app->onAppCmd = previous_command;
    window_app = NULL;
    previous_command = NULL;
    pointer_buttons = 0;
    memset(held, 0, sizeof(held));
    previous_input = NULL;
    close_requested = 0;
}

int
mv_android_window_close_requested(void)
{
    return close_requested;
}

int
mv_android_key_down(MvKey key)
{
    return window_app && (int)key >= 0 && key <= MV_KEY_BACKSPACE && held[key];
}

int
mv_android_mouse_down(MvMouseButton button)
{
    if (!window_app) return 0;
    switch (button) {
    case MV_MOUSE_LEFT: return (pointer_buttons & AMOTION_EVENT_BUTTON_PRIMARY) != 0;
    case MV_MOUSE_RIGHT: return (pointer_buttons & AMOTION_EVENT_BUTTON_SECONDARY) != 0;
    case MV_MOUSE_MIDDLE: return (pointer_buttons & AMOTION_EVENT_BUTTON_TERTIARY) != 0;
    default: return 0;
    }
}
