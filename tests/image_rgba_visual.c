/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine for Lua, inspired by DIV Games Studio.
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
#include "malvado/platform.h"
#include "raylib.h"

int
main(void)
{
    const uint8_t pixels[] = {255, 0, 0, 255, 0, 0, 0, 0,
                              0, 255, 0, 255, 0, 0, 255, 255};
    assert(!mv_image_from_rgba(2, 2, pixels, sizeof(pixels)));
    assert(mv_window_open(64, 64, "RGBA image"));
    assert(!mv_image_from_rgba(0, 2, pixels, sizeof(pixels)));
    assert(!mv_image_from_rgba(INT_MAX, 2, pixels, sizeof(pixels)));
    assert(!mv_image_from_rgba(2, 2, NULL, sizeof(pixels)));
    assert(!mv_image_from_rgba(2, 2, pixels, sizeof(pixels) - 1));
    Image source = {.data = (void *)pixels, .width = 2, .height = 2,
                    .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    int png_size = 0;
    unsigned char *png = ExportImageToMemory(source, ".png", &png_size);
    assert(png && png_size > 24);
    assert(!mv_image_from_png(NULL, (size_t)png_size));
    assert(!mv_image_from_png(png, SIZE_MAX));
    for (int size = 0; size < 24; ++size)
        assert(!mv_image_from_png(png, (size_t)size));
    unsigned char saved = png[0];
    png[0] = 0;
    assert(!mv_image_from_png(png, (size_t)png_size));
    png[0] = saved;
    saved = png[16];
    png[16] = 1;
    assert(!mv_image_from_png(png, (size_t)png_size));
    png[16] = saved;
    MvImage *decoded = mv_image_from_png(png, (size_t)png_size);
    assert(decoded && mv_image_width(decoded) == 2 && mv_image_height(decoded) == 2);
    MemFree(png);
    assert(!mv_image_retain(NULL));
    assert(mv_image_retain(decoded));
    mv_image_destroy(decoded);
    assert(mv_image_width(decoded) == 2);

    MvImage *image = mv_image_from_rgba(2, 2, pixels, sizeof(pixels));
    assert(image && mv_image_width(image) == 2 && mv_image_height(image) == 2);
    mv_window_begin((MvColor){30, 40, 50, 255});
    assert(mv_image_draw_region(decoded, (MvImageRegion){0, 0, 2, 2},
                                (MvImageTransform){10, 10, 0, 0, 10, 0}));
    mv_window_flush();
    Image screen = LoadImageFromScreen();
    assert(screen.data);
    int w = screen.width, h = screen.height;
    Color red = GetImageColor(screen, 15 * w / 64, 15 * h / 64);
    Color clear = GetImageColor(screen, 25 * w / 64, 15 * h / 64);
    Color green = GetImageColor(screen, 15 * w / 64, 25 * h / 64);
    Color blue = GetImageColor(screen, 25 * w / 64, 25 * h / 64);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    assert(clear.r == 30 && clear.g == 40 && clear.b == 50);
    assert(green.g > 240 && green.r < 10 && green.b < 10);
    assert(blue.b > 240 && blue.r < 10 && blue.g < 10);
    UnloadImage(screen);
    mv_window_end();
    mv_image_destroy(image);
    mv_image_destroy(decoded);
    mv_window_close();
    return 0;
}
