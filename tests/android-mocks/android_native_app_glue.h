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
#ifndef MALVADO_TEST_ANDROID_GLUE_H
#define MALVADO_TEST_ANDROID_GLUE_H
#include <stdint.h>
#include <stddef.h>
typedef struct { int type, code, action, source, buttons; } AInputEvent;
struct android_app {
    int32_t (*onInputEvent)(struct android_app *, AInputEvent *);
    void (*onAppCmd)(struct android_app *, int32_t);
};
enum { AINPUT_EVENT_TYPE_MOTION=2, AINPUT_SOURCE_CLASS_POINTER=2,
 AMOTION_EVENT_ACTION_MASK=255, AMOTION_EVENT_ACTION_DOWN=0, AMOTION_EVENT_ACTION_UP=1,
 AMOTION_EVENT_ACTION_CANCEL=3, AMOTION_EVENT_ACTION_BUTTON_PRESS=11, AMOTION_EVENT_ACTION_BUTTON_RELEASE=12,
 AMOTION_EVENT_BUTTON_PRIMARY=1, AMOTION_EVENT_BUTTON_SECONDARY=2, AMOTION_EVENT_BUTTON_TERTIARY=4,
 AINPUT_EVENT_TYPE_KEY=1, AKEY_EVENT_ACTION_DOWN=0, AKEY_EVENT_ACTION_UP=1,
 AKEYCODE_0=7, AKEYCODE_9=16, AKEYCODE_A=29, AKEYCODE_Z=54, AKEYCODE_BACK=4,
 AKEYCODE_ESCAPE=111, AKEYCODE_SPACE=62, AKEYCODE_DPAD_LEFT=21, AKEYCODE_DPAD_RIGHT=22,
 AKEYCODE_DPAD_UP=19, AKEYCODE_DPAD_DOWN=20, AKEYCODE_ENTER=66, AKEYCODE_TAB=61, AKEYCODE_DEL=67,
 APP_CMD_PAUSE=13, APP_CMD_LOST_FOCUS=7, APP_CMD_TERM_WINDOW=2 };
static inline int AInputEvent_getType(AInputEvent *e) { return e->type; }
static inline int AKeyEvent_getKeyCode(AInputEvent *e) { return e->code; }
static inline int AKeyEvent_getAction(AInputEvent *e) { return e->action; }
static inline int AInputEvent_getSource(AInputEvent *e) { return e->source; }
static inline int AMotionEvent_getAction(AInputEvent *e) { return e->action; }
static inline int AMotionEvent_getButtonState(AInputEvent *e) { return e->buttons; }
#endif
