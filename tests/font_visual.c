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
#include <stdlib.h>
#include "malvado/platform.h"
#include "raylib.h"
#include "runner/runtime.h"

typedef struct { Image reference; int matched; } Comparison;

static void
check_frame(void *user)
{
    Comparison *comparison = user;
    Image image = LoadImageFromScreen();
    assert(image.data && image.width == comparison->reference.width && image.height == comparison->reference.height);
    int different = 0;
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            Color a = GetImageColor(image, x, y);
            Color b = GetImageColor(comparison->reference, x, y);
            if (abs((int)a.r - b.r) > 2 || abs((int)a.g - b.g) > 2 || abs((int)a.b - b.b) > 2)
                ++different;
        }
    }
    if (different == 0) comparison->matched = 1;
    UnloadImage(image);
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    const char *path = "tests/fixtures/fonts/AnonymousPro-Bold.ttf";
    assert(!mv_font_load(path, 24));
    assert(mv_window_open(200, 80, "C font test"));
    assert(!mv_font_load("tests/fixtures/fonts/missing.ttf", 24));
    assert(!mv_font_load(path, 0));
    MvFont *otf = mv_font_load("tests/fixtures/fonts/SourceCodePro-Regular.otf", 24);
    assert(otf && mv_font_text_width(otf, "OpenType", 24) > 0);
    mv_font_destroy(otf);
    MvFont *basic = mv_font_load(path, 24);
    assert(basic && mv_font_text_width(basic, "Café", 24) > 0);
    uint32_t glyphs[192];
    size_t count = 0;
    for (uint32_t codepoint = 32; codepoint <= 126; ++codepoint) glyphs[count++] = codepoint;
    for (uint32_t codepoint = 160; codepoint <= 255; ++codepoint) glyphs[count++] = codepoint;
    glyphs[count++] = 0x03a9;
    uint32_t invalid[] = {0xd800};
    uint32_t duplicate[] = {65, 65};
    assert(!mv_font_load_codepoints(path, 24, invalid, 1));
    assert(!mv_font_load_codepoints(path, 24, duplicate, 2));
    assert(!mv_font_load_codepoints(path, 24, NULL, 1));
    MvFont *font = mv_font_load_codepoints(path, 24, glyphs, count);
    assert(font);
    mv_window_begin((MvColor){0, 0, 0, 255});
    assert(mv_font_draw(basic, "Ω", 20, 20, 24, (MvColor){255, 255, 255, 255}));
    mv_window_flush();
    Image fallback = LoadImageFromScreen();
    assert(fallback.data);
    mv_window_end();
    mv_window_begin((MvColor){0, 0, 0, 255});
    assert(mv_font_draw(font, "Ω", 20, 20, 24, (MvColor){255, 255, 255, 255}));
    mv_window_flush();
    Image selected = LoadImageFromScreen();
    assert(selected.data && fallback.width == selected.width && fallback.height == selected.height);
    int glyph_differences = 0;
    for (int y = 0; y < selected.height; ++y) {
        for (int x = 0; x < selected.width; ++x) {
            Color a = GetImageColor(fallback, x, y);
            Color b = GetImageColor(selected, x, y);
            if (abs((int)a.r - b.r) > 2) ++glyph_differences;
        }
    }
    assert(glyph_differences > 10);
    UnloadImage(fallback);
    UnloadImage(selected);
    mv_window_end();
    mv_font_destroy(basic);
    int width = mv_font_text_width(font, "Café Ω", 24);
    assert(width > 0 && !mv_font_text_width(font, NULL, 24));
    mv_window_begin((MvColor){0, 0, 0, 255});
    assert(mv_font_draw(font, "Café Ω", 100 - width / 2, 20, 24, (MvColor){255, 255, 255, 255}));
    mv_window_flush();
    Comparison comparison = {LoadImageFromScreen(), 0};
    assert(comparison.reference.data);
    int light = 0;
    for (int y = 0; y < comparison.reference.height; ++y) {
        for (int x = 0; x < comparison.reference.width; ++x) {
            Color pixel = GetImageColor(comparison.reference, x, y);
            if (pixel.r > 100 && pixel.g > 100 && pixel.b > 100)
                ++light;
        }
    }
    assert(light > 10);
    mv_window_end();
    mv_font_destroy(font);
    mv_font_destroy(NULL);
    mv_window_close();
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &comparison);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    UnloadImage(comparison.reference);
    assert(result == 0 && comparison.matched);
    return 0;
}
