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
#include "malvado/animation.h"
#include "raylib.h"
#include "runner/runtime.h"

static void
check_frame(void *user)
{
    unsigned *seen = user;
    if (*seen == 7) return;
    MalvadoProcessInfo actor;
    size_t count;
    int64_t tick;
    assert(malvado_debug_snapshot(&actor, 1, &count, &tick));
    if (!count) return;
    assert(count == 1);

    MvContext *ctx = malvado_debug_context();
    MvProcessRef process;
    MvAnimationState state;
    MvAnimationSample sample;
    int present;
    assert(mv_process_ref_create(ctx, actor.id, &process) == MV_OK);
    assert(mv_process_animation_clock_get(ctx, process, &state, &sample, &present) == MV_OK);
    if (!present) return;

    /* A render can follow several simulation ticks. Check each sprite on
     * the first render that reaches its interval in the imported sequence. */
    uint64_t elapsed_ms = state.elapsed_us / 1000;
    unsigned frame = 1;
    if (elapsed_ms < 100) frame = 0;
    else if (elapsed_ms >= 300 && elapsed_ms < 600) frame = 2;
    assert(sample.graph_id == (int)frame + 1);
    if (*seen & (1u << frame)) return;
    Image screen = LoadImageFromScreen();
    Image sheet = LoadImage("examples/aseprite/sheet.png");
    assert(screen.data && sheet.data);
    int width = GetScreenWidth(), height = GetScreenHeight();
    Color actual = GetImageColor(screen, 280 * screen.width / width, 190 * screen.height / height);
    int x = frame == 0 ? 2 : 6;
    int y = frame == 2 ? 6 : 2;
    Color expected = GetImageColor(sheet, x, y);
    Color outside = GetImageColor(screen, 240 * screen.width / width, 190 * screen.height / height);
    assert(actual.r == expected.r && actual.g == expected.g && actual.b == expected.b);
    assert(outside.r == 25 && outside.g == 30 && outside.b == 45);
    UnloadImage(sheet);
    UnloadImage(screen);
    *seen |= 1u << frame;
}

int
main(int argc, char **argv)
{
    assert(argc == 2);
    unsigned seen = 0;
    char *args[] = {"malvado-run", argv[1]};
    malvado_debug_set_frame_hook(check_frame, &seen);
    int result = malvado_run(2, args);
    malvado_debug_set_frame_hook(NULL, NULL);
    assert(result == 0 && seen == 7);
    return 0;
}
