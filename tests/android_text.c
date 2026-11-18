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
#include <stdarg.h>
#include <string.h>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
static JNIEnv env;
static int failure, pending, cleared, deleted, attached, detached, calls;
static jint value, env_status, attach_status;

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

static void
clear(JNIEnv *e)
{
    assert(e == &env && pending);
    pending = 0;
    cleared++;
}

static jclass
find(JNIEnv *e, const char *name)
{
    assert(e == &env && !strcmp(name, "android/view/KeyCharacterMap"));
    calls++;
    if (failure == 1) {
        pending = 1;
        return NULL;
    }
    return (void *)1;
}

static jmethodID
load_method(JNIEnv *e, jclass type, const char *name, const char *signature)
{
    assert(e == &env && type == (void *)1);
    assert(!strcmp(name, "load") && !strcmp(signature, "(I)Landroid/view/KeyCharacterMap;"));
    if (failure == 2) {
        pending = 1;
        return NULL;
    }
    return (void *)2;
}

static jobject
load(JNIEnv *e, jclass type, jmethodID id, ...)
{
    assert(e == &env && type == (void *)1 && id == (void *)2);
    va_list args;
    va_start(args, id);
    assert(va_arg(args, int) == 42);
    va_end(args);
    if (failure == 3) {
        pending = 1;
        return NULL;
    }
    return (void *)3;
}

static jmethodID
get_method(JNIEnv *e, jclass type, const char *name, const char *signature)
{
    assert(e == &env && type == (void *)1);
    assert(!strcmp(name, "get") && !strcmp(signature, "(II)I"));
    if (failure == 4) {
        pending = 1;
        return NULL;
    }
    return (void *)4;
}

static jint
get(JNIEnv *e, jobject map, jmethodID id, ...)
{
    assert(e == &env && map == (void *)3 && id == (void *)4);
    va_list args;
    va_start(args, id);
    assert(va_arg(args, int) == 29);
    assert(va_arg(args, int) == 7);
    va_end(args);
    if (failure == 5) pending = 1;
    return value;
}

static void
delete_ref(JNIEnv *e, jobject object)
{
    assert(e == &env && (object == (void *)1 || object == (void *)3));
    deleted++;
}

static void
reset(void)
{
    failure = pending = cleared = deleted = attached = detached = calls = 0;
    env_status = attach_status = JNI_OK;
    value = 'a';
}

int
main(void)
{
    static const struct JNINativeInterface native = {
        .ExceptionCheck = exception,
        .GetMethodID = get_method,
        .DeleteLocalRef = delete_ref,
        .ExceptionClear = clear,
        .FindClass = find,
        .GetStaticMethodID = load_method,
        .CallStaticObjectMethod = load,
        .CallIntMethod = get
    };
    static const struct JNIInvokeInterface invocation = {get_env, attach, detach};
    env = &native;
    JavaVM vm = &invocation;
    ANativeActivity activity = {&vm, (void *)9};
    struct android_app app = {0};
    app.activity = &activity;
    const uint32_t characters[] = {'a', 'Z', 32, 0xe9, 0x1f600, 0x10ffff};
    for (unsigned i = 0; i < sizeof(characters)/sizeof(*characters); i++) {
        reset();
        value = (jint)characters[i];
        assert(mv_android_key_codepoint(&app, 42, 29, 7) == characters[i]);
        assert(deleted == 2 && !detached && !cleared);
    }
    const uint32_t invalid[] = {0, 9, 10, 31, 127, 0x80, 0x9f, 0xd800, 0xdfff, 0x110000, 0x800000b4};
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(*invalid); i++) {
        reset();
        value = (jint)invalid[i];
        assert(!mv_android_key_codepoint(&app, 42, 29, 7));
        assert(deleted == 2);
    }
    for (int step = 1; step <= 5; step++) {
        reset();
        failure = step;
        env_status = JNI_EDETACHED;
        assert(!mv_android_key_codepoint(&app, 42, 29, 7));
        assert(attached == 1 && detached == 1 && cleared == 1 && !pending);
        assert(deleted == (step == 1 ? 0 : step < 4 ? 1 : 2));
    }
    reset();
    env_status = JNI_EDETACHED;
    assert(mv_android_key_codepoint(&app, 42, 29, 7) == 'a');
    assert(attached == 1 && detached == 1);
    reset();
    pending = 1;
    assert(!mv_android_key_codepoint(&app, 42, 29, 7));
    assert(pending && !cleared && !calls);
    reset();
    env_status = JNI_ERR;
    assert(!mv_android_key_codepoint(&app, 42, 29, 7) && !attached);
    reset();
    env_status = JNI_EDETACHED;
    attach_status = JNI_ERR;
    assert(!mv_android_key_codepoint(&app, 42, 29, 7) && !detached);
    assert(!mv_android_key_codepoint(NULL, 42, 29, 7));
    activity.vm = NULL;
    assert(!mv_android_key_codepoint(&app, 42, 29, 7));
}
