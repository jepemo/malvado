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

#include "malvado/fpg.h"
#include <stdlib.h>
#include <string.h>

enum { HEADER_SIZE = 8, PALETTE_SIZE = 768, GAMMA_SIZE = 576, RECORD_SIZE = 64 };
typedef struct {
    int id, width, height;
    size_t record_offset, point_count, pixels_offset;
} Entry;
struct MvFpg {
    uint8_t *bytes;
    Entry *entries;
    size_t count;
};

static uint32_t
read_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int
read_i16(const uint8_t *p)
{
    unsigned value = (unsigned)p[0] | (unsigned)p[1] << 8;
    return value >= 32768 ? (int)value - 65536 : (int)value;
}

static int
validate_records(const uint8_t *data, size_t length, Entry *entries, size_t *total)
{
    size_t offset = HEADER_SIZE + PALETTE_SIZE + GAMMA_SIZE, count = 0;
    unsigned char used[1000] = {0};
    while (offset < length) {
        if (length - offset < RECORD_SIZE || count == 999) return 0;
        const uint8_t *record = data + offset;
        uint32_t id = read_u32(record), size = read_u32(record + 4);
        uint32_t width = read_u32(record + 52), height = read_u32(record + 56);
        uint32_t points = read_u32(record + 60);
        if (id < 1 || id > 999 || used[id] || size != 0 ||
            width < 1 || width > 8192 || height < 1 || height > 8192 || points > 4096)
            return 0;
        used[id] = 1;
        size_t point_bytes = (size_t)points * 4;
        size_t pixels = (size_t)width * height;
        size_t available = length - offset - RECORD_SIZE;
        if (point_bytes > available || pixels > available - point_bytes) return 0;
        if (entries)
            entries[count] = (Entry){(int)id, (int)width, (int)height, offset,
                                     points, offset + RECORD_SIZE + point_bytes};
        ++count;
        offset += RECORD_SIZE + point_bytes + pixels;
    }
    if (!count) return 0;
    *total = count;
    return 1;
}

MvFpg *
mv_fpg_parse(const void *data, size_t length, const char **error)
{
    static const uint8_t magic[8] = {'f', 'p', 'g', 0x1a, 0x0d, 0x0a, 0, 0};
    if (error) *error = "invalid or unsupported 8-bit FPG";
    if (!data || length < HEADER_SIZE + PALETTE_SIZE + GAMMA_SIZE || length > 64U * 1024U * 1024U)
        return NULL;
    const uint8_t *bytes = data;
    if (memcmp(bytes, magic, sizeof(magic))) return NULL;
    for (size_t i = 0; i < PALETTE_SIZE; ++i)
        if (bytes[HEADER_SIZE + i] > 63) return NULL;
    size_t count;
    if (!validate_records(bytes, length, NULL, &count)) return NULL;
    MvFpg *fpg = calloc(1, sizeof(*fpg));
    if (!fpg) return NULL;
    fpg->bytes = malloc(length);
    fpg->entries = calloc(count, sizeof(*fpg->entries));
    if (!fpg->bytes || !fpg->entries) {
        mv_fpg_destroy(fpg);
        return NULL;
    }
    memcpy(fpg->bytes, bytes, length);
    if (!validate_records(fpg->bytes, length, fpg->entries, &fpg->count)) {
        mv_fpg_destroy(fpg);
        return NULL;
    }
    if (error) *error = NULL;
    return fpg;
}

void
mv_fpg_destroy(MvFpg *fpg)
{
    if (!fpg) return;
    free(fpg->entries);
    free(fpg->bytes);
    free(fpg);
}

size_t mv_fpg_count(const MvFpg *fpg) { return fpg ? fpg->count : 0; }
const uint8_t *mv_fpg_palette(const MvFpg *fpg) { return fpg ? fpg->bytes + HEADER_SIZE : NULL; }
const uint8_t *mv_fpg_gamma(const MvFpg *fpg) { return fpg ? fpg->bytes + HEADER_SIZE + PALETTE_SIZE : NULL; }

static const Entry *
find_entry(const MvFpg *fpg, int id)
{
    if (!fpg) return NULL;
    for (size_t i = 0; i < fpg->count; ++i)
        if (fpg->entries[i].id == id) return &fpg->entries[i];
    return NULL;
}

static int
copy_entry(const MvFpg *fpg, const Entry *entry, MvFpgSprite *out)
{
    if (!entry || !out) return 0;
    MvFpgSprite result = {.id = entry->id, .width = entry->width, .height = entry->height,
                          .point_count = entry->point_count, .pixels = fpg->bytes + entry->pixels_offset};
    memcpy(result.name, fpg->bytes + entry->record_offset + 8, sizeof(result.name));
    memcpy(result.filename, fpg->bytes + entry->record_offset + 40, sizeof(result.filename));
    *out = result;
    return 1;
}

int
mv_fpg_sprite(const MvFpg *fpg, int id, MvFpgSprite *out)
{
    return copy_entry(fpg, find_entry(fpg, id), out);
}

int
mv_fpg_at(const MvFpg *fpg, size_t index, MvFpgSprite *out)
{
    if (!fpg || index >= fpg->count) return 0;
    return copy_entry(fpg, &fpg->entries[index], out);
}

int
mv_fpg_point(const MvFpg *fpg, int id, size_t index, MvFpgPoint *out)
{
    const Entry *entry = find_entry(fpg, id);
    if (!entry || !out || index >= entry->point_count) return 0;
    const uint8_t *point = fpg->bytes + entry->record_offset + RECORD_SIZE + index * 4;
    int x = read_i16(point), y = read_i16(point + 2);
    *out = (MvFpgPoint){x, y, !(x == -1 && y == -1)};
    return 1;
}
