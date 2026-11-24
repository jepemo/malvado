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

#include "malvado/platform.h"
#include <android/log.h>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include "raylib.h"
#include "rlgl.h"
#include "lifecycle.h"
#include "audio.h"
#include "failure.h"

int
main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s C entry", ACCEPTANCE_TOKEN);
    lifecycle_begin();
    if (ACCEPTANCE_FAILURE == 3) {
        assert(!mv_window_open(320, 240, "Cancelled startup"));
        struct android_app *app = GetAndroidApp();
        assert(app && app->destroyRequested && !app->window && !app->pendingWindow);
        assert(!IsWindowReady() && !IsAudioDeviceReady());
        __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s startup cancellation passed", ACCEPTANCE_TOKEN);
        return 0;
    }
    if (!mv_window_open(320, 240, "Malvado Android")) return 1;
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s window opened", ACCEPTANCE_TOKEN);
    if (ACCEPTANCE_FAILURE == 1 || ACCEPTANCE_FAILURE == 2) {
        failure_run();
        return 0;
    }
    int width, height;
    assert(mv_window_size(&width, &height) && width > 0 && height > 0);
    audio_begin();
    int verified = 0;
    const char expected_text[] = "aZ0";
    size_t text_length = 0;
    for (int frame = 0; frame < (ACCEPTANCE_LIFECYCLE ? 3600 : 180) && !mv_window_should_close(); ++frame) {
        mv_input_tick();
        if (keyboard_mode()) {
            keyboard_frame();
        }
        if (ACCEPTANCE_LIFECYCLE == 3 || ACCEPTANCE_LIFECYCLE == 7) {
            if (!touch_down && mv_mouse_down(MV_MOUSE_LEFT))
                assert(mv_mouse_pressed(MV_MOUSE_LEFT) && mv_mouse_pressed(MV_MOUSE_LEFT));
            if (touch_down && !touch_released && !mv_mouse_down(MV_MOUSE_LEFT))
                assert(mv_mouse_released(MV_MOUSE_LEFT) && mv_mouse_released(MV_MOUSE_LEFT));
        }
        if (ACCEPTANCE_LIFECYCLE == 9) {
            uint32_t codepoint;
            while (mv_text_input_next(&codepoint)) {
                assert(text_length < sizeof(expected_text) - 1);
                assert(codepoint == (uint32_t)expected_text[text_length++]);
                if (text_length == sizeof(expected_text) - 1) {
                    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance",
                                        "%s c text input passed", ACCEPTANCE_TOKEN);
                }
            }
        }
        mv_window_begin((MvColor){20, 40, 80, 255});
        assert(mv_window_draw_box(width / 4, height / 4, width / 2, height / 2, 1, (MvColor){255, 0, 0, 255}));
        mv_window_flush();
        if (frame == 30) {
            /* Android raylib 5.5 has no DPI query for LoadImageFromScreen.
             * Read the actual render surface rather than the logical window. */
            Image pixels = {0};
            pixels.width = GetRenderWidth();
            pixels.height = GetRenderHeight();
            pixels.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            pixels.mipmaps = 1;
            pixels.data = rlReadScreenPixels(pixels.width, pixels.height);
            assert(pixels.data);
            Color pixel = GetImageColor(pixels, pixels.width / 2, pixels.height / 2);
            assert(pixel.r > 240 && pixel.g < 15 && pixel.b < 15);
            UnloadImage(pixels);
            verified = 1;
        }
        audio_frame();
        lifecycle_frame(verified);
        mv_window_end();
    }
    assert(verified);
    if (ACCEPTANCE_LIFECYCLE == 9) assert(text_length == sizeof(expected_text) - 1);
    if (ACCEPTANCE_LIFECYCLE) assert(mv_window_should_close());
    lifecycle_finish();
    audio_finish();
    mv_window_close();
    lifecycle_cleaned();
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s window pixels and shutdown passed", ACCEPTANCE_TOKEN);
    return 0;
}
