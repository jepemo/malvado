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
    /* Original canvas and crop offset, in pixels. */
    int source_width, source_height, trim_x, trim_y;
} MvAtlasSprite;

/* Parse a schema 1 or 2 atlas and validate regions against the image dimensions.
 * The JSON buffer is borrowed only during this call. The optional error is a
 * static message. The caller owns the result and must destroy it. */
MvAtlas *mv_atlas_parse(const char *json, size_t length, int width, int height, const char **error);
/* Import an Aseprite JSON array export with unrotated frames.
 * The caller supplies the actual image dimensions. The atlas owns its data.
 * Forward, reverse, and ping-pong tags become named animations. Unsupported metadata is
 * rejected; error receives a static message. */
MvAtlas *mv_atlas_parse_aseprite(const char *json, size_t length, int width, int height,
                                 const char **error);
/* Destroy an atlas. A NULL pointer is allowed. */
void mv_atlas_destroy(MvAtlas *atlas);
/* Return a borrowed path valid until atlas destruction, or NULL for NULL atlas. */
const char *mv_atlas_image(const MvAtlas *atlas);
/* Return the sprite count, or 0 for NULL atlas. */
size_t mv_atlas_count(const MvAtlas *atlas);
/* Copy a sprite on success. Return 0 without changing the output for an
 * unknown ID or invalid pointer. The copied name is borrowed from the atlas. */
int mv_atlas_find(const MvAtlas *atlas, int id, MvAtlasSprite *sprite);
/* Find a sprite by its exact name. Leave the output unchanged on failure. */
int mv_atlas_find_name(const MvAtlas *atlas, const char *name, MvAtlasSprite *sprite);
/* Copy the optional color of the imported pivot slice. Return 0
 * without changing out if the slice has no color or the argument is invalid. */
int mv_atlas_pivot_color(const MvAtlas *atlas, MvColor *out);
/* Copy an imported tag color. Return 0 without changing out if absent or invalid. */
int mv_atlas_animation_color(const MvAtlas *atlas, const char *name, MvColor *out);
/* Copy an explicit Aseprite repeat count (0..65535). Return 0 without
 * changing out if the tag is absent or does not specify a count. The count
 * is metadata; mv_atlas_animation_frame does not apply it automatically. */
int mv_atlas_animation_repeats(const MvAtlas *atlas, const char *name, unsigned *out);
/* Find a frame by elapsed milliseconds. Loop must be 0 or 1. Non-looping
 * animations hold their last frame. On failure, the output is unchanged. */
int mv_atlas_animation_frame(const MvAtlas *atlas, const char *name, uint64_t elapsed_ms,
                             int loop, int *sprite_id);

typedef struct { int sprite_id; unsigned duration_ms; } MvAtlasAnimationFrame;
/* Inspect the sequence after expanding tag direction. Count
 * is zero for a missing name. At copies a zero-based entry or returns 0 with
 * unchanged output on an invalid pointer, name or index. */
size_t mv_atlas_animation_frame_count(const MvAtlas *atlas, const char *name);
int mv_atlas_animation_frame_at(const MvAtlas *atlas, const char *name, size_t index,
                                MvAtlasAnimationFrame *out);

#ifdef __cplusplus
}
#endif
#endif
