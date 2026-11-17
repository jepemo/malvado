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
#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION pthread_mutex_t;
static inline int pthread_mutex_init(pthread_mutex_t *m, const void *attr)
{
    (void)attr;
    InitializeCriticalSection(m);
    return 0;
}
static inline int pthread_mutex_destroy(pthread_mutex_t *m) { DeleteCriticalSection(m); return 0; }
static inline int pthread_mutex_lock(pthread_mutex_t *m) { EnterCriticalSection(m); return 0; }
static inline int pthread_mutex_unlock(pthread_mutex_t *m) { LeaveCriticalSection(m); return 0; }
#else
#include <pthread.h>
#endif
/* Only the JNI calls used by the focus adapter are needed in host tests. */
typedef int jint;
typedef unsigned char jboolean;
typedef void *jobject;
typedef jobject jclass;
typedef void *jmethodID;
struct JNINativeInterface;
struct JNIInvokeInterface;
typedef const struct JNINativeInterface *JNIEnv;
typedef const struct JNIInvokeInterface *JavaVM;
struct JNINativeInterface {
    jboolean (*ExceptionCheck)(JNIEnv *);
    jclass (*GetObjectClass)(JNIEnv *, jobject);
    jmethodID (*GetMethodID)(JNIEnv *, jclass, const char *, const char *);
    jboolean (*CallBooleanMethod)(JNIEnv *, jobject, jmethodID, ...);
    void (*DeleteLocalRef)(JNIEnv *, jobject);
    void (*ExceptionClear)(JNIEnv *);
};
struct JNIInvokeInterface {
    jint (*GetEnv)(JavaVM *, void **, jint);
    jint (*AttachCurrentThread)(JavaVM *, JNIEnv **, void *);
    jint (*DetachCurrentThread)(JavaVM *);
};
enum { JNI_OK=0, JNI_ERR=-1, JNI_EDETACHED=-2, JNI_VERSION_1_6=0x00010006, JNI_TRUE=1 };
typedef struct { JavaVM *vm; jobject clazz; } ANativeActivity;
typedef struct { int type, code, action, source, buttons; } AInputEvent;
struct android_app {
    ANativeActivity *activity;
    pthread_mutex_t mutex;
    void *window, *pendingWindow;
    int destroyRequested;
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
 APP_CMD_DESTROY=15, APP_CMD_INIT_WINDOW=1, ALOOPER_POLL_ERROR=-4,
 APP_CMD_GAINED_FOCUS=6, APP_CMD_PAUSE=13, APP_CMD_LOST_FOCUS=7, APP_CMD_TERM_WINDOW=2 };
static inline int AInputEvent_getType(AInputEvent *e) { return e->type; }
static inline int AKeyEvent_getKeyCode(AInputEvent *e) { return e->code; }
static inline int AKeyEvent_getAction(AInputEvent *e) { return e->action; }
static inline int AInputEvent_getSource(AInputEvent *e) { return e->source; }
static inline int AMotionEvent_getAction(AInputEvent *e) { return e->action; }
static inline int AMotionEvent_getButtonState(AInputEvent *e) { return e->buttons; }
struct android_poll_source { void (*process)(struct android_app *, struct android_poll_source *); };
int ALooper_pollOnce(int timeout, int *fd, int *events, void **data);
#endif
