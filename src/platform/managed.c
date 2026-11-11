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

#include "malvado/platform.h"

static void
destroy_image(void *data)
{
    mv_image_destroy(data);
}

MvResult
mv_resource_adopt_image(MvContext *ctx, const MvProcessRef *owner, MvImage *data, MvResource **out)
{
    if (!data) return MV_INVALID_ARGUMENT;
    return mv_resource_create(ctx, owner, data, destroy_image, out);
}

static void
destroy_font(void *data)
{
    mv_font_destroy(data);
}

MvResult
mv_resource_adopt_font(MvContext *ctx, const MvProcessRef *owner, MvFont *data, MvResource **out)
{
    if (!data) return MV_INVALID_ARGUMENT;
    return mv_resource_create(ctx, owner, data, destroy_font, out);
}

static void
destroy_sound(void *data)
{
    mv_sound_destroy(data);
}

MvResult
mv_resource_adopt_sound(MvContext *ctx, const MvProcessRef *owner, MvSound *data, MvResource **out)
{
    if (!data) return MV_INVALID_ARGUMENT;
    return mv_resource_create(ctx, owner, data, destroy_sound, out);
}

static void
destroy_music(void *data)
{
    mv_music_destroy(data);
}

MvResult
mv_resource_adopt_music(MvContext *ctx, const MvProcessRef *owner, MvMusic *data, MvResource **out)
{
    if (!data) return MV_INVALID_ARGUMENT;
    return mv_resource_create(ctx, owner, data, destroy_music, out);
}
