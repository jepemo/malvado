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
