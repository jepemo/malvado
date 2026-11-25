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

#include "malvado/malvado.h"
#include "clock.h"
#include "game_path.h"
#include "process.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct MvContext {
    MalvadoClock clock;
    uint64_t ticks;
    MvProcesses *processes;
    char *game_dir;
    char *assets_dir;
};

MvContext *
mv_context_create(double ticks_per_second, unsigned max_steps)
{
    char *directory = malvado_path_launcher_directory();
    MvContext *ctx = mv_context_create_with_game_dir(ticks_per_second, max_steps, directory);
    free(directory);
    return ctx;
}

MvContext *
mv_context_create_with_game_dir(double ticks_per_second, unsigned max_steps, const char *directory)
{
    MvContext *ctx = malloc(sizeof(*ctx));
    if (!ctx)
        return NULL;
    if (!malvado_clock_init(&ctx->clock, ticks_per_second, max_steps)) {
        free(ctx);
        return NULL;
    }
    ctx->game_dir = directory ? malvado_path_absolute(directory) : malvado_path_executable_directory();
    if (!ctx->game_dir) {
        free(ctx);
        return NULL;
    }
    ctx->assets_dir = NULL;
    ctx->ticks = 0;
    ctx->processes = mv_processes_create(ctx);
    if (!ctx->processes) {
        free(ctx->game_dir);
        free(ctx);
        return NULL;
    }
    return ctx;
}

void
mv_context_destroy(MvContext *ctx)
{
    if (ctx && mv_processes_can_destroy(ctx->processes)) {
        mv_processes_destroy(ctx->processes);
        free(ctx->assets_dir);
        free(ctx->game_dir);
        free(ctx);
    }
}

const char *
mv_context_game_dir(const MvContext *ctx)
{
    return ctx ? ctx->game_dir : NULL;
}

const char *
mv_get_assets_dir(const MvContext *ctx)
{
    return ctx && ctx->assets_dir ? ctx->assets_dir : mv_context_game_dir(ctx);
}

MvResult
mv_set_assets_dir(MvContext *ctx, const char *path)
{
    if (!ctx) return MV_INVALID_ARGUMENT;
    char *directory = NULL;
    if (path && strcmp(path, ".")) {
        size_t size;
        MvResult result = malvado_path_join(ctx->game_dir, path, NULL, 0, &size);
        if (result != MV_OK) return result;
        directory = malloc(size);
        if (!directory) return MV_OUT_OF_MEMORY;
        (void)malvado_path_join(ctx->game_dir, path, directory, size, NULL);
    }
    free(ctx->assets_dir);
    ctx->assets_dir = directory;
    return MV_OK;
}

MvResult
mv_resolve_asset_path(const MvContext *ctx, const char *path, char *out,
                      size_t capacity, size_t *required)
{
    if (!ctx) return MV_INVALID_ARGUMENT;
    return malvado_path_join(mv_get_assets_dir(ctx), path, out, capacity, required);
}

MvProcesses *mv_context_processes(MvContext *ctx) { return ctx ? ctx->processes : NULL; }

MvResult
mv_context_advance(MvContext *ctx, double elapsed_seconds, unsigned *steps)
{
    if (!ctx || !steps || !isfinite(elapsed_seconds) || elapsed_seconds < 0)
        return MV_INVALID_ARGUMENT;

    /* Compute on a copy so a failed operation never changes the context. */
    MalvadoClock clock = ctx->clock;
    unsigned count = malvado_clock_advance(&clock, elapsed_seconds);
    if (!isfinite(clock.remainder) || !isfinite(clock.dropped_seconds) ||
        UINT64_MAX - ctx->ticks < count)
        return MV_OVERFLOW;
    ctx->clock = clock;
    ctx->ticks += count;
    *steps = count;
    return MV_OK;
}

uint64_t
mv_context_ticks(const MvContext *ctx)
{
    return ctx ? ctx->ticks : 0;
}

double
mv_context_dropped_seconds(const MvContext *ctx)
{
    return ctx ? ctx->clock.dropped_seconds : 0;
}
