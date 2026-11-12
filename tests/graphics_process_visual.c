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
#include "malvado/graphic.h"

static MvStepResult native_step(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

static void
check_frame(void *user)
{
    int *frames = user;
    if (++*frames == 1) {
        MvContext *ctx = malvado_debug_context();
        MvProcessId id;
        MvProcessRef ref;
        MvGraphic graph = {.color={255,0,255,255}};
        assert(mv_process_spawn(ctx,0,0,native_step,NULL,NULL,&id) == MV_OK);
        assert(mv_process_ref_create(ctx,id,&ref) == MV_OK);
        assert(mv_graphic_rect(8,8,1,&graph.shape));
        assert(mv_process_graphic_set(ctx,ref,graph) == MV_OK);
        assert(mv_process_set_position(ctx,ref,145,100) == MV_OK);
        assert(mv_process_draw_box(ctx,ref,135,100,4,4,1,(MvColor){0,255,255,255}) == MV_OK);
    }
    if (*frames != 3) return;
    Image image = LoadImageFromScreen();
    assert(image.data);
    int width = GetScreenWidth(), height = GetScreenHeight();
    Color red = GetImageColor(image, 15 * image.width / width, 30 * image.height / height);
    Color blue = GetImageColor(image, 90 * image.width / width, 40 * image.height / height);
    Color green = GetImageColor(image, 80 * image.width / width, 40 * image.height / height);
    Color background = GetImageColor(image, 25 * image.width / width, 30 * image.height / height);
    Color alpha = GetImageColor(image, 130 * image.width / width, 70 * image.height / height);
    Color outside = GetImageColor(image, 119 * image.width / width, 70 * image.height / height);
    Color white = GetImageColor(image, 35 * image.width / width, 90 * image.height / height);
    Color white_outside = GetImageColor(image, 29 * image.width / width, 90 * image.height / height);
    assert(white.r > 240 && white.g > 240 && white.b > 240 && white.a == 255);
    assert(white_outside.r < 10 && white_outside.g < 10 && white_outside.b < 10);
    assert(alpha.r >= 120 && alpha.r <= 135 && alpha.g >= 120 && alpha.g <= 135);
    assert(outside.r < 10 && outside.g < 10 && outside.b < 10);
    assert(red.r > 240 && red.g < 10 && red.b < 10);
    assert(blue.b > 240 && blue.r < 10 && blue.g < 10);
    assert(green.g > 240 && green.r < 10 && green.b < 10);
    assert(background.r < 10 && background.g < 10 && background.b < 10);
    Color native = GetImageColor(image,147*image.width/width,102*image.height/height);
    Color command = GetImageColor(image,136*image.width/width,102*image.height/height);
    assert(native.r > 240 && native.g < 10 && native.b > 240);
    assert(command.r < 10 && command.g > 240 && command.b > 240);
    UnloadImage(image);
    MalvadoProcessInfo processes[6];
    size_t total;
    int64_t tick;
    assert(malvado_debug_snapshot(processes, 6, &total, &tick) && total == 6);
    for (size_t i = 0; i < total; ++i)
        assert(malvado_debug_signal(processes[i].id, "kill"));
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    int frames = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &frames);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && frames >= 3);
    return 0;
}
