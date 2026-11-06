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

#ifdef __cplusplus
}
#endif
#endif
