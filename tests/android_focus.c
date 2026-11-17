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
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

static int env_status, attach_status, attached, detached, pending, cleared;
static int failure, deleted, called, focused;
static JNIEnv env;

static jint
get_env(JavaVM *vm, void **out, jint version)
{
    (void)vm;
    assert(version == JNI_VERSION_1_6);
    *out = &env;
    return env_status;
}

static jint
attach(JavaVM *vm, JNIEnv **out, void *options)
{
    (void)vm;
    assert(!options);
    attached++;
    *out = &env;
    return attach_status;
}

static jint
detach(JavaVM *vm)
{
    (void)vm;
    detached++;
    return JNI_OK;
}

static jboolean
exception(JNIEnv *e)
{
    assert(e == &env);
    return pending != 0;
}

static jclass
object_class(JNIEnv *e, jobject object)
{
    assert(e == &env && object == (void *)1);
    if (failure == 1) { pending = 1; return NULL; }
    return (void *)2;
}

static jmethodID
method(JNIEnv *e, jclass object, const char *name, const char *signature)
{
    assert(e == &env && object == (void *)2);
    assert(!strcmp(name, "hasWindowFocus") && !strcmp(signature, "()Z"));
    if (failure == 2) { pending = 1; return NULL; }
    return (void *)3;
}

static jboolean
call(JNIEnv *e, jobject object, jmethodID id, ...)
{
    assert(e == &env && object == (void *)1 && id == (void *)3);
    called++;
    if (failure == 3) pending = 1;
    return focused != 0;
}

static void
delete_ref(JNIEnv *e, jobject object)
{
    assert(e == &env && object == (void *)2);
    deleted++;
}

static void
clear(JNIEnv *e)
{
    assert(e == &env && pending);
    cleared++;
    pending = 0;
}

static void
reset(void)
{
    env_status = attach_status = JNI_OK;
    attached = detached = pending = cleared = failure = deleted = called = focused = 0;
}

int
main(void)
{
    static const struct JNINativeInterface native = {exception, object_class, method, call, delete_ref, clear};
    static const struct JNIInvokeInterface invocation = {get_env, attach, detach};
    env = &native;
    JavaVM vm = &invocation;
    ANativeActivity activity = {&vm, (void *)1};
    struct android_app app = {0};
    assert(!mv_android_activity_has_focus(NULL));
    assert(!mv_android_activity_has_focus(&app));
    app.activity = &activity;
    for (focused = 0; focused <= 1; ++focused) {
        assert(mv_android_activity_has_focus(&app) == focused);
        assert(!attached && !detached && !cleared);
    }
    assert(called == 2 && deleted == 2);
    for (int step = 1; step <= 3; ++step) {
        reset();
        env_status = JNI_EDETACHED;
        failure = step;
        focused = 1;
        assert(!mv_android_activity_has_focus(&app));
        assert(attached == 1 && detached == 1 && cleared == 1 && !pending);
        assert(deleted == (step != 1));
    }
    reset();
    env_status = JNI_EDETACHED;
    focused = 1;
    assert(mv_android_activity_has_focus(&app));
    assert(attached == 1 && detached == 1 && deleted == 1);
    reset();
    pending = 1;
    assert(!mv_android_activity_has_focus(&app));
    assert(pending && !cleared && !called && !deleted && !detached);
    reset();
    env_status = JNI_ERR;
    assert(!mv_android_activity_has_focus(&app) && !attached && !called);
    env_status = JNI_EDETACHED;
    attach_status = JNI_ERR;
    assert(!mv_android_activity_has_focus(&app) && attached == 1 && !detached && !called);
    reset();
    activity.vm = NULL;
    assert(!mv_android_activity_has_focus(&app));
    activity.vm = &vm;
    activity.clazz = NULL;
    assert(!mv_android_activity_has_focus(&app));
}
