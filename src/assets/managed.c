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

#include "malvado/asset.h"
#include "../core/game_path.h"
#include <stdlib.h>
#include "malvado/atlas.h"

static void
destroy_atlas(void *data)
{
    mv_atlas_destroy(data);
}

MvResult
mv_resource_adopt_atlas(MvContext *ctx, const MvProcessRef *owner, MvAtlas *data, MvResource **out)
{
    if (!data) return MV_INVALID_ARGUMENT;
    return mv_resource_create(ctx, owner, data, destroy_atlas, out);
}

MvAssetSource *
mv_asset_source_open_for_game(const MvContext *ctx, const char *path, const char **error)
{
    char *resolved = malvado_asset_path(ctx, path);
    if (!resolved) {
        if (error) *error = "cannot resolve game asset path";
        return NULL;
    }
    MvAssetSource *source = mv_asset_source_open(resolved, error);
    free(resolved);
    return source;
}
