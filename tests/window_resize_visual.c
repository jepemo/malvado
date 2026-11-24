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


#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "malvado/platform.h"
#include "raylib.h"
#include "runner/runtime.h"

static void
expect_size(int width, int height)
{
    int w, h;
    assert(mv_window_size(&w, &h) && w == width && h == height);
}

static void *
resize_from_wrong_thread(void *unused)
{
    (void)unused;
    assert(!mv_window_set_size(240, 180));
    return NULL;
}

static void
check_native_resize(void)
{
    assert(!mv_window_set_size(160, 120));
    assert(mv_window_open(160, 120, "Native resize test"));
    expect_size(160, 120);
    assert(mv_window_set_title("Renamed native window"));
    GLFWwindow *window = glfwGetCurrentContext();
    assert(window && glfwGetWindowAttrib(window, GLFW_RESIZABLE));
    pthread_t thread;
    assert(pthread_create(&thread, NULL, resize_from_wrong_thread, NULL) == 0);
    assert(pthread_join(thread, NULL) == 0);
    assert(!mv_window_set_size(0, 120) && !mv_window_set_size(160, -1));
    if (glfwGetPlatform() == GLFW_PLATFORM_X11 || glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
        assert(!mv_window_set_size(INT_MAX, 120));
    const MvColor black = {0, 0, 0, 255};
    mv_window_begin(black);
    assert(!mv_window_set_size(200, 140));
    mv_window_end();
    expect_size(160, 120);
    assert(mv_window_set_size(200, 140));
    expect_size(200, 140);
    assert(mv_window_set_size(200, 140));
    assert(glfwGetCurrentContext() == window);
    mv_window_begin(black);
    assert(mv_window_draw_box(10, 10, 10, 10, 1, (MvColor){255, 0, 0, 255}));
    mv_window_end();
    mv_window_close();
    assert(!mv_window_set_size(200, 140));
}

typedef struct {
    GLFWwindow *window;
    int resized_in_init, resized_in_process, sent_external_resize, saw_external_resize;
    int64_t first_external_tick;
} ResizeRun;

static Color
pixel(Image image, int x, int y, int width, int height)
{
    return GetImageColor(image, x * image.width / width, y * image.height / height);
}

static void
check_frame(void *user)
{
    ResizeRun *run = user;
    int width, height, native_width, native_height;
    GLFWwindow *window = glfwGetCurrentContext();
    assert(window);
    if (run->window)
        assert(window == run->window);
    run->window = window;
    assert(mv_window_size(&width, &height));
    glfwGetWindowSize(window, &native_width, &native_height);
    assert(width == native_width && height == native_height);
    assert(!mv_window_set_size(260, 190));
    /* On X11 the logical size can change before raylib receives the
     * framebuffer notification at the end of the frame. */
    if (GetScreenWidth() != width || GetScreenHeight() != height)
        return;
    if (width == 240 && !run->saw_external_resize) {
        /* Let camera follow run a tick with the new dimensions. */
        size_t total;
        int64_t tick;
        assert(malvado_debug_snapshot(NULL, 0, &total, &tick));
        if (!run->first_external_tick)
            run->first_external_tick = tick;
        if (run->first_external_tick == tick)
            return;
    }
    Image image = LoadImageFromScreen();
    assert(image.data);
    Color followed = pixel(image, width / 2, height / 2, width, height);
    Color hud = pixel(image, 19, 11, width, height);
    Color clipped = pixel(image, 23, 11, width, height);
    assert(followed.g > 240 && followed.r < 10);
    assert(hud.r > 240 && hud.g < 10);
    assert(clipped.r < 10 && clipped.g < 10 && clipped.b < 10);
    UnloadImage(image);
    if (width == 200) run->resized_in_init = 1;
    if (width == 220) run->resized_in_process = 1;
    if (width == 240) run->saw_external_resize = 1;
}

static void
resize_outside_runner(void *user)
{
    ResizeRun *run = user;
    if (run->resized_in_process && !run->sent_external_resize) {
        run->sent_external_resize = 1;
        glfwSetWindowSize(run->window, 240, 180);
    }
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    check_native_resize();
    ResizeRun run = {0};
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_hook(resize_outside_runner, &run);
    malvado_debug_set_frame_hook(check_frame, &run);
    int result = malvado_run(2, args);
    malvado_debug_set_hook(NULL, NULL);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && run.resized_in_init && run.resized_in_process && run.saw_external_resize);
    return 0;
}
