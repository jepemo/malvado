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
#include "windows_files.h"
#include <bcrypt.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
file_error(void)
{
    DWORD error = GetLastError();
    errno = error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND ? ENOENT :
            error == ERROR_ALREADY_EXISTS || error == ERROR_FILE_EXISTS ? EEXIST : EACCES;
}

int
malvado_windows_lstat(const char *path, struct stat *info)
{
    DWORD attributes = GetFileAttributesA(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) { file_error(); return -1; }
    if (stat(path, info) != 0) return -1;
    /* Preserve the export/build contract: do not follow links or junctions. */
    if (attributes & FILE_ATTRIBUTE_REPARSE_POINT) info->st_mode = 0;
    return 0;
}

char *
malvado_windows_realpath(const char *path, char *resolved)
{
    HANDLE file = CreateFileA(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (file == INVALID_HANDLE_VALUE) { file_error(); return NULL; }
    char native[MALVADO_CLI_PATH_MAX];
    DWORD length = GetFinalPathNameByHandleA(file, native, sizeof(native), FILE_NAME_NORMALIZED);
    CloseHandle(file);
    if (!length || length >= sizeof(native)) { errno = EINVAL; return NULL; }
    const char *start = native;
    if (!strncmp(start, "\\\\?\\UNC\\", 8)) {
        resolved[0] = '/';
        resolved[1] = '/';
        strcpy(resolved + 2, start + 8);
    } else {
        if (!strncmp(start, "\\\\?\\", 4)) start += 4;
        strcpy(resolved, start);
    }
    for (char *p = resolved; *p; ++p) if (*p == '\\') *p = '/';
    return resolved;
}

MalvadoWindowsDirectory *
malvado_windows_directory_open(const char *path)
{
    size_t size = strlen(path) + 3;
    char *pattern = malloc(size);
    MalvadoWindowsDirectory *directory = calloc(1, sizeof(*directory));
    if (!pattern || !directory) { free(pattern); free(directory); return NULL; }
    snprintf(pattern, size, "%s/*", path);
    directory->handle = FindFirstFileA(pattern, &directory->entry);
    free(pattern);
    if (directory->handle == INVALID_HANDLE_VALUE) { file_error(); free(directory); return NULL; }
    directory->first = 1;
    return directory;
}

const char *
malvado_windows_directory_next(MalvadoWindowsDirectory *directory)
{
    if (directory->first) directory->first = 0;
    else if (!FindNextFileA(directory->handle, &directory->entry)) {
        if (GetLastError() == ERROR_NO_MORE_FILES) errno = 0;
        else file_error();
        return NULL;
    }
    return directory->entry.cFileName;
}

int
malvado_windows_directory_close(MalvadoWindowsDirectory *directory)
{
    int result = FindClose(directory->handle) ? 0 : -1;
    free(directory);
    return result;
}

char *
malvado_windows_stage(const char *parent)
{
    static const char suffix[] = "/.malvado-export-XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    size_t size = strlen(parent) + sizeof(suffix);
    char *stage = malloc(size);
    if (!stage) return NULL;
    snprintf(stage, size, "%s%s", parent, suffix);
    char *token = stage + strlen(stage) - 32;
    static const char hex[] = "0123456789abcdef";
    for (int attempt = 0; attempt < 32; ++attempt) {
        unsigned char bytes[16];
        if (BCryptGenRandom(NULL, bytes, sizeof(bytes), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) break;
        for (size_t i = 0; i < sizeof(bytes); ++i) {
            token[i * 2] = hex[bytes[i] >> 4];
            token[i * 2 + 1] = hex[bytes[i] & 15];
        }
        if (CreateDirectoryA(stage, NULL)) return stage;
        if (GetLastError() != ERROR_ALREADY_EXISTS) { file_error(); free(stage); return NULL; }
    }
    free(stage);
    errno = EIO;
    return NULL;
}

static int
remove_tree(const char *path, unsigned depth)
{
    DWORD attributes = GetFileAttributesA(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) return 0;
    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY)) return DeleteFileA(path) != 0;
    if (attributes & FILE_ATTRIBUTE_REPARSE_POINT) return RemoveDirectoryA(path) != 0;
    if (depth > 64) return 0;
    MalvadoWindowsDirectory *directory = malvado_windows_directory_open(path);
    if (!directory) return 0;
    int ok = 1;
    for (;;) {
        errno = 0;
        const char *name = malvado_windows_directory_next(directory);
        if (!name) { if (errno) ok = 0; break; }
        if (!strcmp(name, ".") || !strcmp(name, "..")) continue;
        size_t size = strlen(path) + strlen(name) + 2;
        char *child = malloc(size);
        if (!child) { ok = 0; break; }
        snprintf(child, size, "%s/%s", path, name);
        ok = remove_tree(child, depth + 1);
        free(child);
        if (!ok) break;
    }
    if (malvado_windows_directory_close(directory) != 0) ok = 0;
    return ok && RemoveDirectoryA(path);
}

int
malvado_windows_remove_tree(const char *path)
{
    return remove_tree(path, 0) ? 0 : -1;
}

int
malvado_windows_publish(const char *stage, const char *output)
{
    if (MoveFileA(stage, output)) return 0;
    file_error();
    return -1;
}
