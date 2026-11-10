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
#include "malvado/fpg.h"
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
_Static_assert(_Generic(&mv_fpg_parse, MvFpg * (*)(const void *, size_t, const char **): 1, default: 0), "mv_fpg_parse signature");
_Static_assert(_Generic(&mv_fpg_destroy, void (*)(MvFpg *): 1, default: 0), "mv_fpg_destroy signature");
_Static_assert(_Generic(&mv_fpg_count, size_t (*)(const MvFpg *): 1, default: 0), "mv_fpg_count signature");
_Static_assert(_Generic(&mv_fpg_palette, const uint8_t * (*)(const MvFpg *): 1, default: 0), "mv_fpg_palette signature");
_Static_assert(_Generic(&mv_fpg_gamma, const uint8_t * (*)(const MvFpg *): 1, default: 0), "mv_fpg_gamma signature");
_Static_assert(_Generic(&mv_fpg_sprite, int (*)(const MvFpg *, int, MvFpgSprite *): 1, default: 0), "mv_fpg_sprite signature");
_Static_assert(_Generic(&mv_fpg_at, int (*)(const MvFpg *, size_t, MvFpgSprite *): 1, default: 0), "mv_fpg_at signature");
_Static_assert(_Generic(&mv_fpg_point, int (*)(const MvFpg *, int, size_t, MvFpgPoint *): 1, default: 0), "mv_fpg_point signature");
_Static_assert(_Generic(&mv_fpg_copy_rgba, int (*)(const MvFpg *, int, int, uint8_t *, size_t): 1, default: 0), "mv_fpg_copy_rgba signature");
_Static_assert(_Generic(&mv_fpg_png_size, size_t (*)(const MvFpg *, int, int): 1, default: 0), "mv_fpg_png_size signature");
_Static_assert(_Generic(&mv_fpg_copy_png, int (*)(const MvFpg *, int, int, uint8_t *, size_t): 1, default: 0), "mv_fpg_copy_png signature");
_Static_assert(_Generic(&mv_fpg_manifest_parse, MvFpgManifest * (*)(const char *, size_t, const char **): 1, default: 0), "mv_fpg_manifest_parse signature");
_Static_assert(_Generic(&mv_fpg_manifest_destroy, void (*)(MvFpgManifest *): 1, default: 0), "mv_fpg_manifest_destroy signature");
_Static_assert(_Generic(&mv_fpg_manifest_count, size_t (*)(const MvFpgManifest *): 1, default: 0), "mv_fpg_manifest_count signature");
_Static_assert(_Generic(&mv_fpg_manifest_transparent_zero, int (*)(const MvFpgManifest *): 1, default: 0), "mv_fpg_manifest_transparent_zero signature");
_Static_assert(_Generic(&mv_fpg_manifest_palette, const uint8_t * (*)(const MvFpgManifest *): 1, default: 0), "mv_fpg_manifest_palette signature");
_Static_assert(_Generic(&mv_fpg_manifest_gamma, const uint8_t * (*)(const MvFpgManifest *): 1, default: 0), "mv_fpg_manifest_gamma signature");
_Static_assert(_Generic(&mv_fpg_manifest_at, int (*)(const MvFpgManifest *, size_t, MvFpgManifestSprite *): 1, default: 0), "mv_fpg_manifest_at signature");
_Static_assert(_Generic(&mv_fpg_manifest_sprite, int (*)(const MvFpgManifest *, int, MvFpgManifestSprite *): 1, default: 0), "mv_fpg_manifest_sprite signature");
_Static_assert(_Generic(&mv_fpg_manifest_point, int (*)(const MvFpgManifest *, int, size_t, MvFpgPoint *): 1, default: 0), "mv_fpg_manifest_point signature");
_Static_assert(_Generic(&mv_fpg_json_size, size_t (*)(const MvFpg *, int): 1, default: 0), "mv_fpg_json_size signature");
_Static_assert(_Generic(&mv_fpg_copy_json, int (*)(const MvFpg *, int, char *, size_t): 1, default: 0), "mv_fpg_copy_json signature");
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
    printf("%zu %zu ", sizeof(MvFpgSprite), _Alignof(MvFpgSprite));
    printf("%zu ", offsetof(MvFpgSprite, id));
    printf("%zu ", offsetof(MvFpgSprite, width));
    printf("%zu ", offsetof(MvFpgSprite, height));
    printf("%zu ", offsetof(MvFpgSprite, name));
    printf("%zu ", offsetof(MvFpgSprite, filename));
    printf("%zu ", offsetof(MvFpgSprite, point_count));
    printf("%zu ", offsetof(MvFpgSprite, pixels));
    printf("%zu %zu ", sizeof(MvFpgPoint), _Alignof(MvFpgPoint));
    printf("%zu ", offsetof(MvFpgPoint, x));
    printf("%zu ", offsetof(MvFpgPoint, y));
    printf("%zu ", offsetof(MvFpgPoint, defined));
    printf("%zu %zu ", sizeof(MvFpgManifestSprite), _Alignof(MvFpgManifestSprite));
    printf("%zu ", offsetof(MvFpgManifestSprite, id));
    printf("%zu ", offsetof(MvFpgManifestSprite, width));
    printf("%zu ", offsetof(MvFpgManifestSprite, height));
    printf("%zu ", offsetof(MvFpgManifestSprite, name));
    printf("%zu ", offsetof(MvFpgManifestSprite, filename));
    printf("%zu ", offsetof(MvFpgManifestSprite, point_count));
    return 0;
}
