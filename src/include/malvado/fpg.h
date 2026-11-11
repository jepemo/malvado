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
#ifndef MALVADO_FPG_H
#define MALVADO_FPG_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MvFpg MvFpg;
typedef struct {
    int id, width, height;
    unsigned char name[32], filename[12];
    size_t point_count;
    const uint8_t *pixels; /* Borrowed; valid until the FPG is destroyed. */
} MvFpgSprite;
typedef struct { int x, y, defined; } MvFpgPoint;

/* Parse the documented BennuGD 8-bit subset. Input is borrowed for this call;
 * the result owns a copy. error, when supplied, receives a static string or
 * NULL on success. Destroy the result with mv_fpg_destroy. */
MvFpg *mv_fpg_parse(const void *data, size_t length, const char **error);
void mv_fpg_destroy(MvFpg *fpg);
size_t mv_fpg_count(const MvFpg *fpg);
/* Borrowed 768-byte six-bit RGB palette and 576-byte gamma block. */
const uint8_t *mv_fpg_palette(const MvFpg *fpg);
const uint8_t *mv_fpg_gamma(const MvFpg *fpg);
/* Both functions return 1 on success; on failure, output is unchanged. */
int mv_fpg_sprite(const MvFpg *fpg, int id, MvFpgSprite *out);
/* Enumerate sprites in file order with a zero-based index. */
int mv_fpg_at(const MvFpg *fpg, size_t index, MvFpgSprite *out);
int mv_fpg_point(const MvFpg *fpg, int id, size_t index, MvFpgPoint *out);
/* Resolve the visual pivot: defined control point 0, otherwise (0, 0).
 * Success writes a defined point. Invalid library/ID/NULL output returns 0
 * without modifying output. No image, context or allocation is needed. */
int mv_fpg_pivot(const MvFpg *fpg, int id, MvFpgPoint *out);

/* Convert to width * height RGBA bytes. Each six-bit channel is shifted left
 * by two. transparent_zero is 0 (opaque) or 1 (index 0 transparent). The
 * output must not overlap FPG data. On invalid input the output is unchanged. */
int mv_fpg_copy_rgba(const MvFpg *fpg, int id, int transparent_zero,
                     uint8_t *output, size_t length);

/* Size and copy an indexed PNG using the original palette and pixel indexes.
 * transparent_zero selects index 0 transparency (0 or 1). A zero size or
 * return value means invalid input. Output must not overlap FPG data; on
 * failure it is unchanged. No gamma correction is applied. */
size_t mv_fpg_png_size(const MvFpg *fpg, int id, int transparent_zero);
int mv_fpg_copy_png(const MvFpg *fpg, int id, int transparent_zero,
                    uint8_t *output, size_t length);

/* Immutable metadata parsed from a version 1 FPG JSON manifest, up to 16 MiB.
 * The parser owns its result and does not retain the input. error receives a
 * static message or NULL on success. Destroy with mv_fpg_manifest_destroy.
 * The manifest has no pixel data and does not load or validate PNG files. */
typedef struct MvFpgManifest MvFpgManifest;
typedef struct {
    int id, width, height;
    unsigned char name[32], filename[12];
    size_t point_count;
} MvFpgManifestSprite;
MvFpgManifest *mv_fpg_manifest_parse(const char *json, size_t length, const char **error);
void mv_fpg_manifest_destroy(MvFpgManifest *manifest);
size_t mv_fpg_manifest_count(const MvFpgManifest *manifest);
/* Returns 0 or 1; -1 means manifest is NULL. */
int mv_fpg_manifest_transparent_zero(const MvFpgManifest *manifest);
const uint8_t *mv_fpg_manifest_palette(const MvFpgManifest *manifest);
const uint8_t *mv_fpg_manifest_gamma(const MvFpgManifest *manifest);
/* Palette and gamma pointers are borrowed. Lookups return 0 without changing
 * the output on invalid inputs or missing entries; indexes are zero-based. */
int mv_fpg_manifest_at(const MvFpgManifest *manifest, size_t index, MvFpgManifestSprite *out);
int mv_fpg_manifest_sprite(const MvFpgManifest *manifest, int id, MvFpgManifestSprite *out);
int mv_fpg_manifest_point(const MvFpgManifest *manifest, int id, size_t index, MvFpgPoint *out);

/* Same pivot contract as mv_fpg_pivot, over converted metadata. */
int mv_fpg_manifest_pivot(const MvFpgManifest *manifest, int id, MvFpgPoint *out);

/* Versioned FPG metadata JSON. Size includes the terminating zero byte.
 * transparent_zero must match PNG encoding. The caller supplies a
 * non-overlapping buffer; failure leaves it unchanged. */
size_t mv_fpg_json_size(const MvFpg *fpg, int transparent_zero);
int mv_fpg_copy_json(const MvFpg *fpg, int transparent_zero, char *output, size_t length);

#ifdef __cplusplus
}
#endif
#endif
