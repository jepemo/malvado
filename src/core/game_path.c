/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "game_path.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#else
#include <unistd.h>
#endif

static int
separator(char c)
{
#ifdef _WIN32
    return c == '/' || c == '\\';
#else
    return c == '/';
#endif
}

#ifdef _WIN32
static int
unc_share(const char *path)
{
    const char *server = path;
    while (*path && !separator(*path)) ++path;
    return path != server && *path && path[1] && !separator(path[1]);
}
#endif

/* Reject Windows paths whose meaning depends on the current drive or its CWD. */
static MvResult
path_kind(const char *path, int *absolute)
{
    if (!path || !*path) return MV_INVALID_ARGUMENT;
    *absolute = separator(path[0]);
#ifdef _WIN32
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, NULL, 0))
        return MV_INVALID_ARGUMENT;
    if (*absolute && !separator(path[1])) return MV_INVALID_ARGUMENT;
    const char *components = path;
    int extended = *absolute && path[2] == '?' && separator(path[3]);
    if (extended) components += 4;
    if (components[0] && components[1] == ':') {
        if (!((components[0] >= 'A' && components[0] <= 'Z') ||
              (components[0] >= 'a' && components[0] <= 'z')) ||
            !separator(components[2])) return MV_INVALID_ARGUMENT;
        *absolute = 1;
        components += 2;
    } else if (*absolute) {
        if (extended) {
            if (_strnicmp(components, "UNC", 3) || !separator(components[3]))
                return MV_INVALID_ARGUMENT;
            components += 4;
        } else {
            components += 2;
        }
        if (!unc_share(components)) return MV_INVALID_ARGUMENT;
    }
    for (const unsigned char *c = (const unsigned char *)components; *c; ++c) {
        if (*c < 32 || strchr("<>:\"|?*", *c)) return MV_INVALID_ARGUMENT;
    }
#endif
    return MV_OK;
}

MvResult
malvado_path_join(const char *base, const char *path, char *out,
                  size_t capacity, size_t *required)
{
    int absolute;
    MvResult result = path_kind(path, &absolute);
    if (result != MV_OK || (!absolute && !base) || (!out && !required))
        return MV_INVALID_ARGUMENT;
    size_t prefix = absolute ? 0 : strlen(base);
    size_t slash = prefix && !separator(base[prefix - 1]);
    size_t length = strlen(path);
    if (prefix > SIZE_MAX - slash || length >= SIZE_MAX - prefix - slash)
        return MV_OVERFLOW;
    size_t size = prefix + slash + length + 1;
    if (required) *required = size;
    if (!out) return MV_OK;
    if (capacity < size) return MV_BUFFER_TOO_SMALL;
    if (prefix) memcpy(out, base, prefix);
    if (slash) out[prefix] = '/';
    memcpy(out + prefix + slash, path, length + 1);
#ifdef _WIN32
    if (prefix >= 4 && separator(base[0]) && separator(base[1]) &&
        base[2] == '?' && separator(base[3])) {
        for (char *part = out + prefix; *part; ++part)
            if (*part == '/') *part = '\\';
    }
#endif
    return MV_OK;
}

#ifdef _WIN32
static char *
utf8_path(const wchar_t *path)
{
    int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, path, -1, NULL, 0, NULL, NULL);
    char *bytes = size > 0 ? malloc((size_t)size) : NULL;
    if (bytes && !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, path, -1, bytes, size, NULL, NULL)) {
        free(bytes);
        return NULL;
    }
    return bytes;
}
#endif

char *
malvado_path_launcher_directory(void)
{
#ifdef _WIN32
    const wchar_t *name = L"MALVADO_GAME_DIR";
    DWORD size = GetEnvironmentVariableW(name, NULL, 0);
    if (!size) return NULL;
    wchar_t *value = malloc((size_t)size * sizeof(*value));
    if (!value) return NULL;
    DWORD length = GetEnvironmentVariableW(name, value, size);
    char *directory = length && length < size ? utf8_path(value) : NULL;
    free(value);
    return directory;
#else
    const char *value = getenv("MALVADO_GAME_DIR");
    return value && *value ? malvado_path_absolute(value) : NULL;
#endif
}

char *
malvado_path_current_directory(void)
{
#ifdef _WIN32
    DWORD size = GetCurrentDirectoryW(0, NULL);
    wchar_t *path = size ? malloc((size_t)size * sizeof(*path)) : NULL;
    if (!path) return NULL;
    DWORD length = GetCurrentDirectoryW(size, path);
    char *bytes = length && length < size ? utf8_path(path) : NULL;
    free(path);
    return bytes;
#else
    for (size_t size = 256; size <= SIZE_MAX / 2; size *= 2) {
        char *path = malloc(size);
        if (!path) return NULL;
        if (getcwd(path, size)) return path;
        int error = errno;
        free(path);
        if (error != ERANGE) return NULL;
    }
    return NULL;
#endif
}

char *
malvado_path_absolute(const char *path)
{
    int absolute;
    if (path_kind(path, &absolute) != MV_OK) return NULL;
    char *cwd = absolute ? NULL : malvado_path_current_directory();
    size_t size;
    char *resolved = NULL;
    if (!absolute && cwd && !strcmp(path, ".")) return cwd;
    if (malvado_path_join(cwd, path, NULL, 0, &size) == MV_OK) {
        resolved = malloc(size);
        if (resolved) (void)malvado_path_join(cwd, path, resolved, size, NULL);
    }
    free(cwd);
    return resolved;
}

char *
malvado_path_parent(const char *path)
{
    size_t end = strlen(path);
    while (end && !separator(path[end - 1])) --end;
    if (!end) return NULL;
    if (end > 1 && !separator(path[end - 2])) {
#ifdef _WIN32
        if (!(end == 3 && path[1] == ':'))
#endif
            --end;
    }
    char *parent = malloc(end + 1);
    if (parent) {
        memcpy(parent, path, end);
        parent[end] = '\0';
    }
    return parent;
}

char *
malvado_path_executable_directory(void)
{
#ifdef _WIN32
    wchar_t executable[32768];
    DWORD length = GetModuleFileNameW(NULL, executable, sizeof(executable) / sizeof(*executable));
    if (!length || length >= sizeof(executable) / sizeof(*executable)) return NULL;
    char *path = utf8_path(executable);
#elif defined(__APPLE__)
    uint32_t size = 0;
    (void)_NSGetExecutablePath(NULL, &size);
    char *executable = size ? malloc(size) : NULL;
    if (!executable) return NULL;
    char *path = _NSGetExecutablePath(executable, &size) == 0 ?
                 malvado_path_absolute(executable) : NULL;
    free(executable);
#else
    char *path = NULL;
    for (size_t size = 256; size <= SIZE_MAX / 2; size *= 2) {
        path = malloc(size + 1);
        if (!path) return NULL;
        ssize_t length = readlink("/proc/self/exe", path, size);
        if (length >= 0 && (size_t)length < size) {
            path[length] = '\0';
            break;
        }
        free(path);
        path = NULL;
        if (length < 0) return NULL;
    }
#endif
    char *directory = path ? malvado_path_parent(path) : NULL;
    free(path);
    return directory;
}

char *
malvado_asset_path(const MvContext *ctx, const char *path)
{
    size_t size;
    if (mv_resolve_asset_path(ctx, path, NULL, 0, &size) != MV_OK) return NULL;
    char *resolved = malloc(size);
    if (resolved) (void)mv_resolve_asset_path(ctx, path, resolved, size, NULL);
    return resolved;
}
