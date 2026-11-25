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
#include "../core/game_path.h"
#include <stdlib.h>

MvImage *
mv_image_load_for_game(const MvContext *ctx, const char *path)
{
    char *resolved = malvado_asset_path(ctx, path);
    if (!resolved) return NULL;
    MvImage *resource = mv_image_load(resolved);
    free(resolved);
    return resource;
}

MvSound *
mv_sound_load_for_game(const MvContext *ctx, const char *path)
{
    char *resolved = malvado_asset_path(ctx, path);
    if (!resolved) return NULL;
    MvSound *resource = mv_sound_load(resolved);
    free(resolved);
    return resource;
}

MvMusic *
mv_music_load_for_game(const MvContext *ctx, const char *path, int loop)
{
    char *resolved = malvado_asset_path(ctx, path);
    if (!resolved) return NULL;
    MvMusic *resource = mv_music_load(resolved, loop);
    free(resolved);
    return resource;
}

MvFont *
mv_font_load_for_game(const MvContext *ctx, const char *path, int base_size)
{
    char *resolved = malvado_asset_path(ctx, path);
    if (!resolved) return NULL;
    MvFont *resource = mv_font_load(resolved, base_size);
    free(resolved);
    return resource;
}

MvFont *
mv_font_load_codepoints_for_game(const MvContext *ctx, const char *path, int base_size, const uint32_t *codepoints, size_t count)
{
    char *resolved = malvado_asset_path(ctx, path);
    if (!resolved) return NULL;
    MvFont *resource = mv_font_load_codepoints(resolved, base_size, codepoints, count);
    free(resolved);
    return resource;
}
