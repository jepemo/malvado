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

extern struct android_app *GetAndroidApp(void);
static int verified;
static int lua_touch_seen;
static unsigned lua_keyboard_seen;
#include "lifecycle.h"

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
    if (ACCEPTANCE_LIFECYCLE == 3 && touch_released && !lua_touch_seen) {
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
    struct android_app *app = GetAndroidApp();
    assert(app && app->activity);
    assert(chdir(app->activity->internalDataPath) == 0);
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
    if (ACCEPTANCE_LIFECYCLE == 3) assert(setenv("MALVADO_APK_TOUCH", "1", 1) == 0);
    if (keyboard_mode()) {
        assert(copy_asset(app->activity->assetManager, "keyboard.lua"));
        assert(setenv("MALVADO_APK_KEYBOARD", "1", 1) == 0);
    }
    assert(freopen("stderr.txt", "w", stderr));
    malvado_debug_set_frame_hook(check_frame, NULL);
    char *arguments[] = {"game", "main.lua"};
    int result = malvado_run(2, arguments);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == (ACCEPTANCE_FAILURE ? 1 : 0));
    lifecycle_finish();
    if (keyboard_mode()) assert(lua_keyboard_seen == 3);
    assert(!IsAudioDeviceReady());
    if (ACCEPTANCE_AUDIO) {
        FILE *audio = fopen("audio.txt", "rb");
        assert(audio && fgetc(audio) == '1' && fgetc(audio) == EOF);
        assert(fclose(audio) == 0);
        __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s audio API passed", ACCEPTANCE_TOKEN);
    }
    assert(!IsWindowReady() && malvado_debug_context() == NULL);
    assert(ACCEPTANCE_FAILURE == 2 || verified);
    FILE *closed = fopen("closed.txt", "rb");
    assert(closed && fgetc(closed) == '1' && fgetc(closed) == EOF);
    assert(fclose(closed) == 0);
    if (ACCEPTANCE_LIFECYCLE == 3) {
        FILE *touch = fopen("touch.txt", "rb");
        assert(touch && fgetc(touch) == '1' && fgetc(touch) == EOF);
        assert(fclose(touch) == 0);
    }
    if (ACCEPTANCE_FAILURE) {
        assert(fflush(stderr) == 0);
        FILE *errors = fopen("stderr.txt", "rb");
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
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s lua assets and shutdown passed", ACCEPTANCE_TOKEN);
    return 0;
}
