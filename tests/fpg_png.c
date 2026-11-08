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

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "malvado/fpg.h"

int
main(int argc, char **argv)
{
    assert(argc == 3);
#ifdef _WIN32
    assert(_setmode(_fileno(stdout), _O_BINARY) != -1);
#endif
    FILE *file = fopen(argv[1], "rb");
    assert(file);
    size_t capacity = 70000;
    unsigned char *data = calloc(capacity, 1);
    assert(data);
    size_t length = fread(data, 1, capacity, file);
    assert(!ferror(file));
    fclose(file);
    int large = !strcmp(argv[2], "large");
    if (large) {
        /* Keep the header and first record; use a 257 x 257 indexed image. */
        size_t record = 1352, pixels = record + 64 + 8;
        data[record + 52] = 1; data[record + 53] = 1;
        data[record + 56] = 1; data[record + 57] = 1;
        for (size_t i = 0; i < 257U * 257U; ++i) data[pixels + i] = (unsigned char)i;
        length = pixels + 257U * 257U;
    }
    const char *error = NULL;
    MvFpg *fpg = mv_fpg_parse(data, length, &error);
    assert(fpg && !error);
    if (!strcmp(argv[2], "json") || !strcmp(argv[2], "json-opaque")) {
        int transparent = strcmp(argv[2], "json-opaque") != 0;
        size_t size = mv_fpg_json_size(fpg, transparent);
        char *json = malloc(size);
        assert(json && mv_fpg_copy_json(fpg, transparent, json, size));
        assert(fwrite(json, 1, size - 1, stdout) == size - 1);
        free(json);
        mv_fpg_destroy(fpg);
        free(data);
        return 0;
    }
    int transparent = strcmp(argv[2], "opaque") != 0;
    size_t size = mv_fpg_png_size(fpg, 7, transparent);
    unsigned char *png = malloc(size);
    assert(png && mv_fpg_copy_png(fpg, 7, transparent, png, size));
    assert(fwrite(png, 1, size, stdout) == size);
    free(png);
    mv_fpg_destroy(fpg);
    free(data);
    return 0;
}
