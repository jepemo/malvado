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
#include <stdio.h>
_Static_assert(_Generic(&mv_atlas_parse, MvAtlas * (*)(const char *, size_t, int, int, const char **): 1, default: 0), "mv_atlas_parse signature");
_Static_assert(_Generic(&mv_atlas_parse_aseprite, MvAtlas * (*)(const char *, size_t, int, int, const char **): 1, default: 0), "mv_atlas_parse_aseprite signature");
_Static_assert(_Generic(&mv_atlas_destroy, void (*)(MvAtlas *): 1, default: 0), "mv_atlas_destroy signature");
_Static_assert(_Generic(&mv_atlas_image, const char * (*)(const MvAtlas *): 1, default: 0), "mv_atlas_image signature");
_Static_assert(_Generic(&mv_atlas_count, size_t (*)(const MvAtlas *): 1, default: 0), "mv_atlas_count signature");
_Static_assert(_Generic(&mv_atlas_find, int (*)(const MvAtlas *, int, MvAtlasSprite *): 1, default: 0), "mv_atlas_find signature");
_Static_assert(_Generic(&mv_atlas_find_name, int (*)(const MvAtlas *, const char *, MvAtlasSprite *): 1, default: 0), "mv_atlas_find_name signature");
_Static_assert(_Generic(&mv_atlas_pivot_color, int (*)(const MvAtlas *, MvColor *): 1, default: 0), "mv_atlas_pivot_color signature");
_Static_assert(_Generic(&mv_atlas_animation_color, int (*)(const MvAtlas *, const char *, MvColor *): 1, default: 0), "mv_atlas_animation_color signature");
_Static_assert(_Generic(&mv_atlas_animation_repeats, int (*)(const MvAtlas *, const char *, unsigned *): 1, default: 0), "mv_atlas_animation_repeats signature");
_Static_assert(_Generic(&mv_atlas_animation_frame, int (*)(const MvAtlas *, const char *, uint64_t, int, int *): 1, default: 0), "mv_atlas_animation_frame signature");
_Static_assert(_Generic(&mv_atlas_animation_frame_count, size_t (*)(const MvAtlas *, const char *): 1, default: 0), "mv_atlas_animation_frame_count signature");
_Static_assert(_Generic(&mv_atlas_animation_frame_at, int (*)(const MvAtlas *, const char *, size_t, MvAtlasAnimationFrame *): 1, default: 0), "mv_atlas_animation_frame_at signature");
int main(void)
{
    printf("%zu %zu ", sizeof(MvColor), _Alignof(MvColor));
    printf("%zu ", offsetof(MvColor, r));
    printf("%zu ", offsetof(MvColor, g));
    printf("%zu ", offsetof(MvColor, b));
    printf("%zu ", offsetof(MvColor, a));
    printf("%zu %zu ", sizeof(MvImageRegion), _Alignof(MvImageRegion));
    printf("%zu ", offsetof(MvImageRegion, x));
    printf("%zu ", offsetof(MvImageRegion, y));
    printf("%zu ", offsetof(MvImageRegion, width));
    printf("%zu ", offsetof(MvImageRegion, height));
    printf("%zu %zu ", sizeof(MvAtlasSprite), _Alignof(MvAtlasSprite));
    printf("%zu ", offsetof(MvAtlasSprite, id));
    printf("%zu ", offsetof(MvAtlasSprite, name));
    printf("%zu ", offsetof(MvAtlasSprite, region));
    printf("%zu ", offsetof(MvAtlasSprite, pivot_x));
    printf("%zu ", offsetof(MvAtlasSprite, pivot_y));
    printf("%zu ", offsetof(MvAtlasSprite, source_width));
    printf("%zu ", offsetof(MvAtlasSprite, source_height));
    printf("%zu ", offsetof(MvAtlasSprite, trim_x));
    printf("%zu ", offsetof(MvAtlasSprite, trim_y));
    printf("%zu %zu ", sizeof(MvAtlasAnimationFrame), _Alignof(MvAtlasAnimationFrame));
    printf("%zu ", offsetof(MvAtlasAnimationFrame, sprite_id));
    printf("%zu ", offsetof(MvAtlasAnimationFrame, duration_ms));
    return 0;
}
