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
#include <stdio.h>
#include <stdarg.h>
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

int
mv_fpg_copy_rgba(const MvFpg *fpg, int id, int transparent_zero,
                 uint8_t *output, size_t length)
{
    const Entry *entry = find_entry(fpg, id);
    if (!entry || !output || (transparent_zero != 0 && transparent_zero != 1))
        return 0;
    size_t count = (size_t)entry->width * (size_t)entry->height;
    if (length / 4 < count) return 0;
    const uint8_t *pixels = fpg->bytes + entry->pixels_offset;
    const uint8_t *palette = mv_fpg_palette(fpg);
    for (size_t i = 0; i < count; ++i) {
        unsigned index = pixels[i];
        output[4 * i] = (uint8_t)(palette[3 * index] << 2);
        output[4 * i + 1] = (uint8_t)(palette[3 * index + 1] << 2);
        output[4 * i + 2] = (uint8_t)(palette[3 * index + 2] << 2);
        output[4 * i + 3] = (uint8_t)(transparent_zero && index == 0 ? 0 : 255);
    }
    return 1;
}

static void
write_be32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24); p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8); p[3] = (uint8_t)value;
}

static uint32_t
png_crc(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xffffffffU;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xedb88320U & (uint32_t)-(int)(crc & 1));
    }
    return crc ^ 0xffffffffU;
}

static uint8_t *
png_chunk(uint8_t *out, const char type[4], const uint8_t *data, size_t length)
{
    write_be32(out, (uint32_t)length);
    memcpy(out + 4, type, 4);
    if (length) memcpy(out + 8, data, length);
    write_be32(out + 8 + length, png_crc(out + 4, length + 4));
    return out + length + 12;
}

size_t
mv_fpg_png_size(const MvFpg *fpg, int id, int transparent_zero)
{
    const Entry *entry = find_entry(fpg, id);
    if (!entry || (transparent_zero != 0 && transparent_zero != 1)) return 0;
    size_t raw = ((size_t)entry->width + 1) * (size_t)entry->height;
    size_t blocks = (raw + 65534) / 65535;
    /* Signature, IHDR, PLTE, optional tRNS, IDAT, IEND and zlib stream. */
    return 8 + 25 + 780 + (transparent_zero ? 13 : 0) + 12 + 6 + raw + blocks * 5 + 12;
}

int
mv_fpg_copy_png(const MvFpg *fpg, int id, int transparent_zero,
                uint8_t *output, size_t length)
{
    const Entry *entry = find_entry(fpg, id);
    size_t needed = mv_fpg_png_size(fpg, id, transparent_zero);
    if (!entry || !output || !needed || length < needed) return 0;
    static const uint8_t signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    memcpy(output, signature, 8);
    uint8_t header[13] = {0};
    write_be32(header, (uint32_t)entry->width);
    write_be32(header + 4, (uint32_t)entry->height);
    header[8] = 8; header[9] = 3;
    uint8_t *out = png_chunk(output + 8, "IHDR", header, sizeof(header));
    uint8_t palette[768];
    const uint8_t *source_palette = mv_fpg_palette(fpg);
    for (size_t i = 0; i < sizeof(palette); ++i) palette[i] = (uint8_t)(source_palette[i] << 2);
    out = png_chunk(out, "PLTE", palette, sizeof(palette));
    if (transparent_zero) {
        const uint8_t alpha = 0;
        out = png_chunk(out, "tRNS", &alpha, 1);
    }
    size_t raw = ((size_t)entry->width + 1) * (size_t)entry->height;
    size_t blocks = (raw + 65534) / 65535;
    size_t compressed = 6 + raw + blocks * 5;
    write_be32(out, (uint32_t)compressed);
    memcpy(out + 4, "IDAT", 4);
    uint8_t *chunk = out + 4;
    out += 8;
    *out++ = 0x78; *out++ = 0x01;
    uint32_t a = 1, b = 0;
    size_t position = 0;
    while (position < raw) {
        size_t remaining = raw - position;
        uint16_t count = (uint16_t)(remaining > 65535 ? 65535 : remaining);
        *out++ = remaining <= 65535 ? 1 : 0;
        *out++ = (uint8_t)count; *out++ = (uint8_t)(count >> 8);
        uint16_t inverse = (uint16_t)~count;
        *out++ = (uint8_t)inverse; *out++ = (uint8_t)(inverse >> 8);
        for (unsigned i = 0; i < count; ++i, ++position) {
            size_t row_width = (size_t)entry->width + 1;
            size_t col = position % row_width;
            uint8_t value = col ? fpg->bytes[entry->pixels_offset + position / row_width * (size_t)entry->width + col - 1] : 0;
            *out++ = value;
            a = (a + value) % 65521; b = (b + a) % 65521;
        }
    }
    write_be32(out, b << 16 | a); out += 4;
    write_be32(out, png_crc(chunk, compressed + 4)); out += 4;
    png_chunk(out, "IEND", NULL, 0);
    return 1;
}

typedef struct { char *output; size_t length; } JsonWriter;

static void
json_text(JsonWriter *writer, const char *format, ...)
{
    char text[160];
    va_list arguments;
    va_start(arguments, format);
    int length = vsnprintf(text, sizeof(text), format, arguments);
    va_end(arguments);
    if (writer->output) memcpy(writer->output + writer->length, text, (size_t)length);
    writer->length += (size_t)length;
}

static void
json_hex(JsonWriter *writer, const uint8_t *data, size_t length)
{
    static const char hex[] = "0123456789abcdef";
    if (writer->output) {
        for (size_t i = 0; i < length; ++i) {
            writer->output[writer->length + i * 2] = hex[data[i] >> 4];
            writer->output[writer->length + i * 2 + 1] = hex[data[i] & 15];
        }
    }
    writer->length += length * 2;
}

static size_t
fpg_json(const MvFpg *fpg, int transparent_zero, char *output)
{
    JsonWriter w = {output, 0};
    json_text(&w, "{\"schema\":1,\"kind\":\"malvado-fpg-8\",\"transparent_zero\":%s,\"palette_hex\":\"", transparent_zero ? "true" : "false");
    json_hex(&w, mv_fpg_palette(fpg), PALETTE_SIZE);
    json_text(&w, "\",\"gamma_hex\":\"");
    json_hex(&w, mv_fpg_gamma(fpg), GAMMA_SIZE);
    json_text(&w, "\",\"sprites\":[");
    for (size_t i = 0; i < fpg->count; ++i) {
        const Entry *entry = &fpg->entries[i];
        const uint8_t *record = fpg->bytes + entry->record_offset;
        json_text(&w, "%s{\"id\":%d,\"image\":\"%d.png\",\"width\":%d,\"height\":%d,\"name_hex\":\"", i ? "," : "", entry->id, entry->id, entry->width, entry->height);
        json_hex(&w, record + 8, 32);
        json_text(&w, "\",\"filename_hex\":\"");
        json_hex(&w, record + 40, 12);
        json_text(&w, "\",\"points\":[");
        for (size_t j = 0; j < entry->point_count; ++j) {
            const uint8_t *p = record + RECORD_SIZE + j * 4;
            int x = read_i16(p), y = read_i16(p + 2);
            json_text(&w, "%s{\"x\":%d,\"y\":%d,\"defined\":%s}", j ? "," : "", x, y, x == -1 && y == -1 ? "false" : "true");
        }
        json_text(&w, "]}");
    }
    json_text(&w, "]}");
    if (output) output[w.length] = '\0';
    return w.length + 1;
}

size_t
mv_fpg_json_size(const MvFpg *fpg, int transparent_zero)
{
    if (!fpg || (transparent_zero != 0 && transparent_zero != 1)) return 0;
    return fpg_json(fpg, transparent_zero, NULL);
}

int
mv_fpg_copy_json(const MvFpg *fpg, int transparent_zero, char *output, size_t length)
{
    size_t needed = mv_fpg_json_size(fpg, transparent_zero);
    if (!needed || !output || length < needed) return 0;
    fpg_json(fpg, transparent_zero, output);
    return 1;
}

int
mv_fpg_pivot(const MvFpg *library, int id, MvFpgPoint *out)
{
    MvFpgSprite sprite;
    if (!out || !mv_fpg_sprite(library, id, &sprite))
        return 0;
    MvFpgPoint point = {0};
    (void)mv_fpg_point(library, id, 0, &point);
    if (!point.defined)
        point.x = point.y = 0;
    point.defined = 1;
    *out = point;
    return 1;
}
