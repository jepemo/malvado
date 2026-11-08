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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define make_directory(path) _mkdir(path)
#define remove_directory(path) _rmdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define make_directory(path) mkdir(path, 0777)
#define remove_directory(path) rmdir(path)
#endif
#include "malvado/fpg.h"
#include "fpg_export.h"

static char *
output_path(const char *directory, const char *name)
{
    size_t left = strlen(directory), right = strlen(name);
    if (left > SIZE_MAX - right - 2) return NULL;
    char *path = malloc(left + right + 2);
    if (path) snprintf(path, left + right + 2, "%s/%s", directory, name);
    return path;
}

static int
write_file(const char *directory, const char *name, const void *bytes, size_t length)
{
    char *path = output_path(directory, name);
    if (!path) return 0;
    FILE *file = fopen(path, "wbx");
    if (!file) {
        perror(path);
        free(path);
        return 0;
    }
    int ok = fwrite(bytes, 1, length, file) == length;
    if (fclose(file) != 0) ok = 0;
    if (!ok) {
        fprintf(stderr, "Cannot write %s\n", path);
        if (remove(path) != 0) perror(path);
    }
    free(path);
    return ok;
}

static void
remove_sprite(const char *directory, int id)
{
    char name[32];
    snprintf(name, sizeof(name), "%d.png", id);
    char *path = output_path(directory, name);
    if (!path) {
        fprintf(stderr, "Cannot clean up %s/%s\n", directory, name);
        return;
    }
    if (remove(path) != 0) perror(path);
    free(path);
}

int
malvado_fpg_export(const char *source, const char *destination, int transparent_zero)
{
    FILE *file = fopen(source, "rb");
    if (!file) { perror(source); return 1; }
    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return 1; }
    long size = ftell(file);
    if (size < 0 || size > 64L * 1024 * 1024 || fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Invalid or oversized FPG input\n");
        fclose(file);
        return 1;
    }
    unsigned char *input = malloc(size ? (size_t)size : 1);
    if (!input) { fclose(file); return 1; }
    int ok = fread(input, 1, (size_t)size, file) == (size_t)size;
    if (fclose(file) != 0) ok = 0;
    const char *error = NULL;
    MvFpg *fpg = ok ? mv_fpg_parse(input, (size_t)size, &error) : NULL;
    free(input);
    if (!fpg) {
        fprintf(stderr, "Cannot read FPG: %s\n", error ? error : "input error");
        return 1;
    }
    if (make_directory(destination) != 0) {
        perror(destination);
        mv_fpg_destroy(fpg);
        return 1;
    }
    size_t written = 0;
    for (size_t i = 0; i < mv_fpg_count(fpg); ++i) {
        MvFpgSprite sprite;
        if (!mv_fpg_at(fpg, i, &sprite)) break;
        size_t length = mv_fpg_png_size(fpg, sprite.id, transparent_zero);
        unsigned char *png = malloc(length);
        char name[32];
        snprintf(name, sizeof(name), "%d.png", sprite.id);
        ok = png && mv_fpg_copy_png(fpg, sprite.id, transparent_zero, png, length) &&
             write_file(destination, name, png, length);
        free(png);
        if (!ok) break;
        ++written;
    }
    if (written == mv_fpg_count(fpg)) {
        size_t length = mv_fpg_json_size(fpg, transparent_zero);
        char *json = malloc(length);
        ok = json && mv_fpg_copy_json(fpg, transparent_zero, json, length) &&
             write_file(destination, "manifest.json", json, length - 1);
        free(json);
    } else ok = 0;
    if (!ok) {
        fprintf(stderr, "FPG export failed; removing incomplete output\n");
        for (size_t i = 0; i < written; ++i) {
            MvFpgSprite sprite;
            if (mv_fpg_at(fpg, i, &sprite)) remove_sprite(destination, sprite.id);
        }
        if (remove_directory(destination) != 0) perror(destination);
    }
    mv_fpg_destroy(fpg);
    return ok ? 0 : 1;
}
