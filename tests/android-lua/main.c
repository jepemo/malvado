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

#include "runner/runtime.h"
#include "malvado/asset.h"
#include "raylib.h"
#include "rlgl.h"
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <android/log.h>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

extern struct android_app *GetAndroidApp(void);
static int verified;
static int lua_touch_seen, lua_touch_held, lua_touch_moved;
static unsigned lua_keyboard_seen, lua_keyboard_held;
static unsigned keyboard_ticks, keyboard_frame_ticks;
static int keyboard_stall, keyboard_catchup;
#include "lifecycle.h"
static int lua_text_seen;

/* raylib wraps fopen through its live APK manager. After window destruction,
 * use ordinary sandbox descriptors for the fixture cleanup evidence. */
static FILE *
read_private(struct android_app *app, const char *name)
{
    size_t size = strlen(app->activity->internalDataPath) + strlen(name) + 2;
    char *path = malloc(size);
    assert(path);
    snprintf(path, size, "%s/%s", app->activity->internalDataPath, name);
    int descriptor = open(path, O_RDONLY);
    FILE *file = descriptor >= 0 ? fdopen(descriptor, "rb") : NULL;
    if (descriptor >= 0 && !file) close(descriptor);
    free(path);
    return file;
}

static void
check_tick(void *user)
{
    (void)user;
    keyboard_ticks++;
    keyboard_frame();
}

/* The fixture copies a fixed allowlist into its own application sandbox.
 * This is not the general APK asset service or a public extraction API. */
static int
copy_asset(AAssetManager *manager, const char *name)
{
    AAsset *asset = AAssetManager_open(manager, name, AASSET_MODE_BUFFER);
    if (!asset) return 0;
    off_t size = AAsset_getLength(asset);
    assert(size >= 0 && size <= 4 * 1024 * 1024);
    const void *bytes = AAsset_getBuffer(asset);
    FILE *file = fopen(name, "wb");
    assert(file && bytes);
    assert(fwrite(bytes, 1, (size_t)size, file) == (size_t)size);
    assert(fclose(file) == 0);
    AAsset_close(asset);
    return 1;
}

static void
check_frame(void *user)
{
    (void)user;
    /* Simulation ticks and presented frames do not have a one-to-one cadence.
     * Startup frames can precede the first process draw command. Require the
     * expected pixel before termination, without assuming a render count. */
    lifecycle_frame(verified);
    if (ACCEPTANCE_LIFECYCLE == 9 && !lua_text_seen) {
        FILE *file = fopen("text-input.txt", "rb");
        if (file) {
            char text[4] = {0};
            assert(fread(text, 1, sizeof(text), file) == 3 && strcmp(text, "aZ0") == 0);
            assert(fclose(file) == 0);
            lua_text_seen = 1;
            __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance",
                                "%s lua text input passed", ACCEPTANCE_TOKEN);
        }
    }
    if (keyboard_mode()) {
        if (keyboard_stall && !keyboard_catchup && keyboard_ticks - keyboard_frame_ticks > 1) {
            keyboard_catchup = 1;
            __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s lua keyboard catch-up passed", ACCEPTANCE_TOKEN);
        }
        keyboard_frame_ticks = keyboard_ticks;
        /* Force multiple logical ticks after a render frame. A render-only
         * observer can legitimately miss the first tick's input edge. */
        if (!keyboard_stall && mv_key_down(MV_KEY_LEFT)) {
            keyboard_stall = 1;
            mv_platform_wait(0.1);
        }
    }
    if (keyboard_mode() && lua_keyboard_held < 2) {
        FILE *file = fopen("keyboard-held.txt", "rb");
        if (file) {
            char state[3] = {0};
            size_t count = fread(state, 1, sizeof(state), file);
            assert(count <= 2 && memcmp(state, "12", count) == 0);
            assert(fclose(file) == 0);
            while (lua_keyboard_held < count) {
                __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s lua keyboard %s held",
                                    ACCEPTANCE_TOKEN, lua_keyboard_held == 0 ? "a" : "left");
                lua_keyboard_held++;
            }
        }
    }
    if (keyboard_mode() && lua_keyboard_seen < 3) {
        FILE *keyboard = fopen("keyboard.txt", "rb");
        if (keyboard) {
            char state[4] = {0};
            size_t count = fread(state, 1, sizeof(state), keyboard);
            assert(count <= 3 && memcmp(state, "123", count) == 0);
            assert(fclose(keyboard) == 0);
            const char *messages[] = {"a released", "left released", "passed"};
            while (lua_keyboard_seen < count) {
                __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s lua keyboard %s",
                                    ACCEPTANCE_TOKEN, messages[lua_keyboard_seen]);
                lua_keyboard_seen++;
            }
        }
    }
    if (ACCEPTANCE_LIFECYCLE == 3 || ACCEPTANCE_LIFECYCLE == 7) {
        const char *names[] = {"touch-held.txt", "touch-moved.txt"};
        int *seen[] = {&lua_touch_held, &lua_touch_moved};
        for (unsigned i = 0; i < 2; ++i) {
            FILE *file = !*seen[i] ? fopen(names[i], "rb") : NULL;
            if (file) {
                assert(fgetc(file) == '1' && fgetc(file) == EOF);
                assert(fclose(file) == 0);
                *seen[i] = 1;
                __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s lua touch %s",
                                    ACCEPTANCE_TOKEN, i == 0 ? "held" : "moved");
            }
        }
    }
    if ((ACCEPTANCE_LIFECYCLE == 3 || ACCEPTANCE_LIFECYCLE == 7) && touch_released && !lua_touch_seen) {
        FILE *touch = fopen("touch.txt", "rb");
        if (touch) {
            assert(fgetc(touch) == '1' && fgetc(touch) == EOF);
            assert(fclose(touch) == 0);
            lua_touch_seen = 1;
            __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s lua touch released", ACCEPTANCE_TOKEN);
        }
    }
    if (verified) return;
    mv_window_flush();
    Image image = {0};
    image.width = GetRenderWidth();
    image.height = GetRenderHeight();
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    image.data = rlReadScreenPixels(image.width, image.height);
    assert(image.data);
    Color pixel = GetImageColor(image, image.width / 2, image.height / 2);
    verified = pixel.r > 240 && pixel.g < 15 && pixel.b < 15;
    UnloadImage(image);
    lifecycle_frame(verified);
}

int
main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    lifecycle_begin();
    verified = 0;
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s Lua entry", ACCEPTANCE_TOKEN);
    struct android_app *app = GetAndroidApp();
    assert(app && app->activity);
    assert(chdir(app->activity->internalDataPath) == 0);
    if (ACCEPTANCE_LIFECYCLE == 8) {
        /* Each runtime must produce its own exactly-once cleanup evidence. */
        remove("closed.txt");
        remove("audio.txt");
    }
    const char *required[] = {"main.lua", "behavior.lua", "atlas.json", "message.bin"};
    for (size_t i = 0; i < sizeof(required) / sizeof(required[0]); ++i)
        assert(copy_asset(app->activity->assetManager, required[i]));
    if (ACCEPTANCE_AUDIO) {
        const char *audio[] = {"audio.lua", "silence.wav", "silence.ogg", "empty.wav"};
        for (unsigned i = 0; i < 4; ++i) assert(copy_asset(app->activity->assetManager, audio[i]));
        assert(setenv("MALVADO_APK_AUDIO", "1", 1) == 0);
    }
    int image = copy_asset(app->activity->assetManager, "red.png");
    assert(image == (ACCEPTANCE_FAILURE != 2));
    MvAssetSource *snapshot = mv_asset_source_open("message.bin", NULL);
    assert(snapshot);
    if (ACCEPTANCE_FAILURE == 1) assert(setenv("MALVADO_APK_FAIL", "1", 1) == 0);
    if (ACCEPTANCE_LIFECYCLE) assert(setenv("MALVADO_APK_LIFECYCLE", "1", 1) == 0);
    if (ACCEPTANCE_LIFECYCLE == 9) assert(setenv("MALVADO_APK_TEXT", "1", 1) == 0);
    if (ACCEPTANCE_LIFECYCLE == 3 || ACCEPTANCE_LIFECYCLE == 7) assert(setenv("MALVADO_APK_TOUCH", "1", 1) == 0);
    if (keyboard_mode()) {
        assert(copy_asset(app->activity->assetManager, "keyboard.lua"));
        assert(setenv("MALVADO_APK_KEYBOARD", "1", 1) == 0);
    }
    assert(freopen("stderr.txt", "w", stderr));
    malvado_debug_set_frame_hook(check_frame, NULL);
    if (keyboard_mode()) malvado_debug_set_hook(check_tick, NULL);
    char *arguments[] = {"game", "main.lua"};
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s Lua runtime starting", ACCEPTANCE_TOKEN);
    int result = malvado_run(2, arguments);
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s Lua runtime stopped", ACCEPTANCE_TOKEN);
    malvado_debug_set_frame_hook(NULL, NULL);
    if (keyboard_mode()) malvado_debug_set_hook(NULL, NULL);
    assert(result == (ACCEPTANCE_FAILURE ? 1 : 0));
    if (ACCEPTANCE_FAILURE == 3) {
        assert(app->destroyRequested && !app->window && !app->pendingWindow);
        assert(!verified && !IsWindowReady() && !IsAudioDeviceReady());
        assert(malvado_debug_context() == NULL);
        assert(access("closed.txt", F_OK) != 0);
        assert(fflush(stderr) == 0);
        FILE *errors = read_private(app, "stderr.txt");
        char text[8192] = {0};
        assert(errors && fread(text, 1, sizeof(text) - 1, errors) > 0);
        assert(fclose(errors) == 0 && strstr(text, "could not open window"));
        mv_asset_source_destroy(snapshot);
        __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s startup cancellation passed", ACCEPTANCE_TOKEN);
        return 0;
    }
    lifecycle_finish();
    if (ACCEPTANCE_LIFECYCLE == 9) assert(lua_text_seen);
    if (keyboard_mode()) assert(lua_keyboard_seen == 3 && keyboard_stall && keyboard_catchup);
    assert(!IsAudioDeviceReady());
    if (ACCEPTANCE_AUDIO) {
        FILE *audio = read_private(app, "audio.txt");
        assert(audio && fgetc(audio) == '1' && fgetc(audio) == EOF);
        assert(fclose(audio) == 0);
        __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s audio API passed", ACCEPTANCE_TOKEN);
    }
    assert(!IsWindowReady() && malvado_debug_context() == NULL);
    assert(ACCEPTANCE_FAILURE == 2 || verified);
    FILE *closed = read_private(app, "closed.txt");
    assert(closed && fgetc(closed) == '1' && fgetc(closed) == EOF);
    assert(fclose(closed) == 0);
    if (ACCEPTANCE_LIFECYCLE == 3 || ACCEPTANCE_LIFECYCLE == 7) {
        FILE *touch = read_private(app, "touch.txt");
        assert(touch && fgetc(touch) == '1' && fgetc(touch) == EOF);
        assert(fclose(touch) == 0);
    }
    if (ACCEPTANCE_FAILURE) {
        assert(fflush(stderr) == 0);
        FILE *errors = read_private(app, "stderr.txt");
        assert(errors);
        char text[8192] = {0};
        assert(fread(text, 1, sizeof(text) - 1, errors) > 0);
        assert(fclose(errors) == 0);
        assert(strstr(text, ACCEPTANCE_FAILURE == 1 ? "APK_EXPECTED_PROCESS_FAILURE" : "could not load PNG"));
    }
    assert(remove("message.bin") == 0);
    size_t length;
    const unsigned char *bytes = mv_asset_source_data(snapshot, &length);
    const unsigned char expected[] = {0, 13, 10, 255};
    assert(length == sizeof(expected) && !memcmp(bytes, expected, length));
    mv_asset_source_destroy(snapshot);
    lifecycle_cleaned();
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s lua assets and shutdown passed", ACCEPTANCE_TOKEN);
    return 0;
}
