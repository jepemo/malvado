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
#include "raylib.h"
#include "runner/runtime.h"
#include "malvado/platform.h"

static Color pixel(Image image, int x, int y)
{
    return GetImageColor(image, x * image.width / GetScreenWidth(),
                        y * image.height / GetScreenHeight());
}

static void check_frame(void *user)
{
    unsigned *checked = user;
    MalvadoProcessInfo processes[3];
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 3, &count, &tick));
    if (tick < 10) return;
    unsigned stage = tick < 22 ? 1 : 2;
    if (*checked & stage) return;
    assert(count == 3);
    for (size_t i=1; i<count; ++i) {
        MvProcessRef ref;
        MvImageGraphic selected;
        int present;
        assert(mv_process_ref_create(malvado_debug_context(),processes[i].id,&ref) == MV_OK);
        assert(mv_process_image_get(malvado_debug_context(),ref,&selected,&present) == MV_OK && present);
        assert(mv_process_animation_clock_clear(malvado_debug_context(),ref) == MV_BUSY);
    }

    Image screen = LoadImageFromScreen();
    Image sheet = LoadImage("examples/aseprite/sheet.png");
    assert(screen.data && sheet.data);
    Color a = pixel(screen, 30, 20), b = pixel(screen, 70, 20);
    Color expected_a = GetImageColor(sheet, 6, stage == 1 ? 2 : 6);
    Color expected_b = GetImageColor(sheet, stage == 1 ? 2 : 6, 2);
    assert(a.r == expected_a.r && a.g == expected_a.g && a.b == expected_a.b);
    assert(b.r == expected_b.r && b.g == expected_b.g && b.b == expected_b.b);
    UnloadImage(sheet);
    UnloadImage(screen);
    *checked |= stage;
    if (*checked == 3)
        for (size_t i = count; i > 0; --i)
            assert(malvado_debug_signal(processes[i - 1].id, "kill"));
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    unsigned checked = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &checked);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && checked == 3);
    return 0;
}
