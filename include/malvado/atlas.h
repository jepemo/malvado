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
#ifndef MALVADO_ATLAS_H
#define MALVADO_ATLAS_H

#include <stddef.h>
#include "malvado/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MvAtlas MvAtlas;
typedef struct {
    int id;
    const char *name; /* Borrowed; valid until the atlas is destroyed. */
    MvImageRegion region;
    int pivot_x, pivot_y;
} MvAtlasSprite;

/* Parse a schema 1 atlas and validate regions against the image dimensions.
 * The JSON buffer is borrowed only during this call. The optional error is a
 * static message. The caller owns the result and must destroy it. */
MvAtlas *mv_atlas_parse(const char *json, size_t length, int width, int height, const char **error);
void mv_atlas_destroy(MvAtlas *atlas);
const char *mv_atlas_image(const MvAtlas *atlas);
size_t mv_atlas_count(const MvAtlas *atlas);
/* Return 1 and copy a sprite on success, or 0 if the ID is not present. */
int mv_atlas_find(const MvAtlas *atlas, int id, MvAtlasSprite *sprite);

#ifdef __cplusplus
}
#endif
#endif
