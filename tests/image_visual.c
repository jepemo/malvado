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

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include "malvado/platform.h"
#include "raylib.h"
#include "runner/runtime.h"

static Image reference;

static Color
pixel(Image screen, int x, int y)
{
    return GetImageColor(screen, x * screen.width / GetScreenWidth(),
                         y * screen.height / GetScreenHeight());
}

static void
check_pixels(void)
{
    Image screen = LoadImageFromScreen();
    assert(screen.data);
    int width = GetScreenWidth(), height = GetScreenHeight();
    Color red = GetImageColor(screen, 12 * screen.width / width, 12 * screen.height / height);
    Color background = GetImageColor(screen, 30 * screen.width / width, 30 * screen.height / height);
    Color green = GetImageColor(screen, 44 * screen.width / width, 40 * screen.height / height);
    Color green_default = GetImageColor(screen, 26 * screen.width / width, 6 * screen.height / height);
    Color outside = GetImageColor(screen, 50 * screen.width / width, 46 * screen.height / height);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    assert(background.r < 10 && background.g < 10 && background.b < 10);
    assert(green.r < 10 && green.g > 240 && green.b < 10);
    assert(green_default.r < 10 && green_default.g > 240 && green_default.b < 10);
    assert(outside.r < 10 && outside.g < 10 && outside.b < 10);
    assert(pixel(screen, 4, 19).g == 255 && pixel(screen, 3, 19).g == 0);
    assert(pixel(screen, 21, 30).g == 255 && pixel(screen, 22, 30).g == 0);
    assert(pixel(screen, 9, 35).b == 255);
    assert(pixel(screen, 10, 35).b == 0 && pixel(screen, 9, 36).b == 0);
    Color right = pixel(screen, 24, 50), bottom = pixel(screen, 20, 54);
    assert(right.r >= 120 && right.r <= 135 && bottom.r == right.r);
    assert(pixel(screen, 25, 50).r == 0 && pixel(screen, 20, 55).r == 0);
    assert(pixel(screen, 20, 50).r == 0 && pixel(screen, 12, 45).b == 255);
    Color center = pixel(screen, 35, 50);
    assert(center.r == 0 && center.g == 255 && center.b == 255);
    assert(pixel(screen, 55, 15).r == 0 && pixel(screen, 41, 50).g == 0);
    int outline_pixels = 0;
    for (int y = 13; y <= 17; ++y) {
        for (int x = 58; x <= 59; ++x) {
            Color edge = pixel(screen, x, y);
            if (edge.r == 255 && edge.g == 0 && edge.b == 255)
                ++outline_pixels;
        }
    }
    assert(outline_pixels > 0);
    UnloadImage(screen);
}

static void
check_frame(void *user)
{
    int *frames = user;
    check_pixels();
    Image screen = LoadImageFromScreen();
    assert(screen.data && screen.width == reference.width && screen.height == reference.height);
    for (int y = 0; y < screen.height; ++y) {
        for (int x = 0; x < screen.width; ++x) {
            Color a = GetImageColor(reference, x, y);
            Color b = GetImageColor(screen, x, y);
            assert(abs((int)a.r - b.r) <= 2 && abs((int)a.g - b.g) <= 2 &&
                   abs((int)a.b - b.b) <= 2);
        }
    }
    UnloadImage(screen);
    ++*frames;
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    assert(!mv_image_load("tests/fixtures/red.png"));
    assert(mv_window_open(64, 64, "C image test"));
    assert(!mv_image_load("tests/fixtures/missing.png"));
    assert(!mv_image_load("tests/fixtures/red.txt"));
    MvImage *image = mv_image_load("tests/fixtures/red.png");
    assert(image && mv_image_width(image) == 8 && mv_image_height(image) == 8);
    MvImageRegion region = {4, 0, 4, 8};
    MvImageTransform transform = {50, 40, 2, 4, 2, 90};
    assert(mv_image_region_valid(image, region, transform));
    assert(!mv_image_region_valid(image, (MvImageRegion){7, 0, 2, 1}, transform));
    assert(!mv_image_region_valid(image, region, (MvImageTransform){0, 0, 0, 0, 0, 0}));
    assert(!mv_image_region_valid(NULL, region, transform));
    mv_window_begin((MvColor){0, 0, 0, 255});
    mv_image_draw(image, 10, 10);
    assert(mv_image_draw_region(image, region, (MvImageTransform){25, 5, 0, 0, 1, 0}));
    assert(mv_image_draw_region(image, region, transform));
    MvImageRegion odd = {4, 0, 3, 3};
    assert(mv_image_draw_region(image, odd, (MvImageTransform){8.5f, 23.5f, 1.5f, 1.5f, 3, 0}));
    assert(mv_image_draw_region(image, odd, (MvImageTransform){23.5f, 23.5f, 0.5f, -0.5f, 3, 90}));
    assert(!mv_image_draw_region(image, (MvImageRegion){0, 0, 9, 8}, transform));
    assert(mv_window_draw_box(2, 30, 8, 6, 1, (MvColor){0, 0, 255, 255}));
    assert(mv_window_draw_box(15, 45, 10, 10, 0, (MvColor){255, 255, 0, 128}));
    assert(mv_window_draw_box(12, 45, 1, 1, 0, (MvColor){0, 0, 255, 255}));
    assert(mv_window_draw_circle(35, 50, 4, 1, (MvColor){0, 255, 255, 255}));
    assert(mv_window_draw_circle(55, 15, 4, 0, (MvColor){255, 0, 255, 255}));
    assert(!mv_window_draw_box(0, 0, 0, 2, 1, (MvColor){0}));
    assert(!mv_window_draw_box(INT_MAX, 0, 2, 1, 1, (MvColor){0}));
    assert(!mv_window_draw_box(0, 0, 1, 1, 2, (MvColor){0}));
    assert(!mv_window_draw_circle(0, 0, -1, 1, (MvColor){0}));
    assert(!mv_window_draw_circle(INT_MIN, 0, 1, 1, (MvColor){0}));
    mv_window_flush();
    check_pixels();
    reference = LoadImageFromScreen();
    assert(reference.data);
    mv_window_end();
    mv_image_destroy(image);
    mv_image_destroy(NULL);
    mv_window_close();

    int frames = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &frames);
    int status = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(status == 0 && frames > 0);
    UnloadImage(reference);
    return 0;
}
