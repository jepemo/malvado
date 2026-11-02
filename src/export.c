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

#if !defined(_WIN32) && defined(MALVADO_EXPORT_GRAPHICS)
#define _XOPEN_SOURCE 700
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#include "export.h"

static char *
join_path(const char *directory, const char *name)
{
    size_t size = strlen(directory) + strlen(name) + 2;
    char *result = malloc(size);
    if (result) snprintf(result, size, "%s/%s", directory, name);
    return result;
}

static int
copy_file(const char *source, const char *destination, int executable)
{
    FILE *input = fopen(source, "rb");
    if (!input) { perror(source); return 0; }
    FILE *output = fopen(destination, "wx");
    if (!output) { perror(destination); fclose(input); return 0; }
    char buffer[16384];
    size_t count;
    int ok = 1;
    for (;;) {
        count = fread(buffer, 1, sizeof(buffer), input);
        if (count && fwrite(buffer, 1, count, output) != count) { ok = 0; break; }
        if (count < sizeof(buffer)) break;
    }
    if (ferror(input)) ok = 0;
    if (fclose(input) != 0) ok = 0;
    if (fclose(output) != 0) ok = 0;
    if (ok && executable && chmod(destination, 0755) != 0) ok = 0;
    if (!ok) fprintf(stderr, "Cannot copy %s\n", source);
    return ok;
}

static int
copy_tree(const char *source, const char *destination, unsigned depth, int native)
{
    if (depth > 64) { fprintf(stderr, "Project is too deep\n"); return 0; }
    DIR *dir = opendir(source);
    if (!dir) { perror(source); return 0; }
    int ok = 1;
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(dir);
        if (!entry) { if (errno) ok = 0; break; }
        const char *name = entry->d_name;
        if (name[0] == '.') continue;
        size_t name_length = strlen(name);
        if (native && (!strcmp(name, "CMakeLists.txt") ||
            (name_length >= 2 && (!strcmp(name + name_length - 2, ".c") ||
                                  !strcmp(name + name_length - 2, ".h"))))) continue;
        if (depth == 0 && (!strcmp(name, "game") || !strcmp(name, "game-bin") || !strcmp(name, "licenses"))) {
            fprintf(stderr, "Reserved project name: %s\n", name); ok = 0; break;
        }
        char *from = join_path(source, name);
        char *to = destination ? join_path(destination, name) : NULL;
        if (!from || (destination && !to)) { free(from); free(to); ok = 0; break; }
        struct stat info;
        if (lstat(from, &info) != 0) { perror(from); ok = 0; }
        else if (S_ISDIR(info.st_mode) && (!strcmp(name, "build") || !strcmp(name, "dist"))) {
            /* Build outputs are not game assets. */
        } else if (S_ISREG(info.st_mode)) {
            if (destination) ok = copy_file(from, to, 0);
        } else if (S_ISDIR(info.st_mode)) {
            if (destination && mkdir(to, 0755) != 0) { perror(to); ok = 0; }
            else ok = copy_tree(from, to, depth + 1, native);
        } else {
            fprintf(stderr, "Unsupported project entry: %s\n", from); ok = 0;
        }
        free(from); free(to);
        if (!ok) break;
    }
    if (closedir(dir) != 0) ok = 0;
    return ok;
}

static int
executable_directory(char *path, size_t size)
{
#ifdef __APPLE__
    uint32_t length = (uint32_t)size;
    if (_NSGetExecutablePath(path, &length) != 0) return 0;
#else
    ssize_t length = readlink("/proc/self/exe", path, size - 1);
    if (length <= 0 || (size_t)length >= size - 1) return 0;
    path[length] = '\0';
#endif
    char *slash = strrchr(path, '/');
    if (!slash) return 0;
    *slash = '\0';
    return 1;
}

int
malvado_sdk_root(char *path, size_t size)
{
    char executable[PATH_MAX], resolved[PATH_MAX];
    if (!path || !size || !executable_directory(executable, sizeof(executable))) return 0;
    char *root = join_path(executable, MALVADO_SDK_ROOT_REL);
    if (!root) return 0;
    int ok = realpath(root, resolved) != NULL && strlen(resolved) < size;
    free(root);
    if (ok) memcpy(path, resolved, strlen(resolved) + 1);
    return ok;
}

int
malvado_export_status(void)
{
    char executable[PATH_MAX];
    if (!executable_directory(executable, sizeof(executable))) return 1;
    char *launcher = join_path(executable, MALVADO_LAUNCHER_REL);
    char *native_launcher = join_path(executable, MALVADO_NATIVE_LAUNCHER_REL);
    char *notices = join_path(executable, MALVADO_NOTICES_REL);
    const char *required[] = {"LICENSE", "Lua-readme.html", "raylib-LICENSE",
#ifdef MALVADO_HAS_ASSET_NOTICES
        "cJSON-LICENSE",
#endif
    };
    struct stat info;
    int ready = launcher && native_launcher && notices &&
                stat(launcher, &info) == 0 && S_ISREG(info.st_mode) && access(launcher, R_OK | X_OK) == 0 &&
                stat(native_launcher, &info) == 0 && S_ISREG(info.st_mode) && access(native_launcher, R_OK | X_OK) == 0 &&
                stat(notices, &info) == 0 && S_ISDIR(info.st_mode);
    for (size_t i = 0; ready && i < sizeof(required) / sizeof(required[0]); ++i) {
        char *notice = join_path(notices, required[i]);
        ready = notice && stat(notice, &info) == 0 && S_ISREG(info.st_mode) && access(notice, R_OK) == 0;
        free(notice);
    }
    free(launcher);
    free(native_launcher);
    free(notices);
    return ready ? 2 : 1;
}

static int
valid_bundle_name(const unsigned char *name, size_t length)
{
    if (!length) return 0;
    for (size_t i = 0; i < length;) {
        unsigned char first = name[i++];
        unsigned code = first;
        unsigned more = 0, minimum = 0;
        if (first < 0x80) {
            if (first < 0x20) return 0;
        } else if (first >= 0xc2 && first <= 0xdf) {
            code = first & 0x1f; more = 1; minimum = 0x80;
        } else if (first >= 0xe0 && first <= 0xef) {
            code = first & 0x0f; more = 2; minimum = 0x800;
        } else if (first >= 0xf0 && first <= 0xf4) {
            code = first & 0x07; more = 3; minimum = 0x10000;
        } else return 0;
        if (more > length - i) return 0;
        for (unsigned j = 0; j < more; ++j) {
            unsigned char next = name[i++];
            if ((next & 0xc0) != 0x80) return 0;
            code = (code << 6) | (next & 0x3f);
        }
        if (code < minimum || code > 0x10ffff || (code >= 0xd800 && code <= 0xdfff) ||
            code == 0xfffe || code == 0xffff) return 0;
    }
    return 1;
}

static int
write_bundle_info(const char *path, const char *name, size_t length)
{
    const char prefix[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
        "<plist version=\"1.0\"><dict>\n"
        "<key>CFBundleExecutable</key><string>game</string>\n"
        "<key>CFBundleIdentifier</key><string>org.malvado.game</string>\n"
        "<key>CFBundleName</key><string>";
    const char suffix[] = "</string>\n"
        "<key>CFBundlePackageType</key><string>APPL</string>\n"
        "<key>CFBundleVersion</key><string>1</string>\n"
        "<key>CFBundleShortVersionString</key><string>1.0</string>\n"
        "<key>NSHighResolutionCapable</key><true/>\n"
        "</dict></plist>\n";
    FILE *file = fopen(path, "wx");
    if (!file) { perror(path); return 0; }
    int ok = fwrite(prefix, 1, sizeof(prefix) - 1, file) == sizeof(prefix) - 1;
    for (size_t i = 0; ok && i < length; ++i) {
        const char *escaped = name[i] == '&' ? "&amp;" : name[i] == '<' ? "&lt;" : name[i] == '>' ? "&gt;" : NULL;
        if (escaped) ok = fputs(escaped, file) >= 0;
        else ok = fputc((unsigned char)name[i], file) != EOF;
    }
    if (ok) ok = fwrite(suffix, 1, sizeof(suffix) - 1, file) == sizeof(suffix) - 1;
    if (fclose(file) != 0) ok = 0;
    if (!ok) fprintf(stderr, "Cannot write %s\n", path);
    return ok;
}

int
malvado_export(const char *output, int native, int app)
{
#ifndef __APPLE__
    if (app) { fprintf(stderr, "App export requires macOS\n"); return 1; }
#endif
    if (app && (!output || strlen(output) < 4 || strcmp(output + strlen(output) - 4, ".app") != 0)) {
        fprintf(stderr, "App export destination must end in .app\n"); return 1;
    }
    const char *app_name = app ? strrchr(output, '/') : NULL;
    if (app) app_name = app_name ? app_name + 1 : output;
    size_t app_name_length = app ? strlen(app_name) - 4 : 0;
    if (app && !valid_bundle_name((const unsigned char *)app_name, app_name_length)) {
        fprintf(stderr, "App name must be non-empty valid UTF-8 without control characters\n"); return 1;
    }
    char current[PATH_MAX], executable[PATH_MAX];
    if (!output || !*output || !realpath(".", current) || !executable_directory(executable, sizeof(executable))) {
        fprintf(stderr, "Cannot determine project or SDK location\n"); return 1;
    }
    char *parent = strdup(output);
    if (!parent) return 1;
    char *slash = strrchr(parent, '/');
    if (slash) { if (slash == parent) slash[1] = '\0'; else *slash = '\0'; }
    else strcpy(parent, ".");
    char resolved_parent[PATH_MAX];
    if (!realpath(parent, resolved_parent)) { perror(parent); free(parent); return 1; }
    free(parent);
    size_t project_length = strlen(current);
    char *dist = join_path(current, "dist");
    if (!dist) return 1;
    int in_dist = !strcmp(dist, resolved_parent);
    free(dist);
    if (!strcmp(current, "/") || (!in_dist && !strncmp(current, resolved_parent, project_length) &&
        (resolved_parent[project_length] == '\0' || resolved_parent[project_length] == '/'))) {
        fprintf(stderr, "Export destination must be in dist or outside the project directory\n"); return 1;
    }
    struct stat info;
    const char *entry = native ? "main.c" : "main.lua";
    if (lstat(entry, &info) != 0 || !S_ISREG(info.st_mode)) {
        fprintf(stderr, "Project needs a regular %s file\n", entry); return 1;
    }
    char *launcher = join_path(executable, native ? MALVADO_NATIVE_LAUNCHER_REL : MALVADO_LAUNCHER_REL);
    char *notices = join_path(executable, MALVADO_NOTICES_REL);
    if (!launcher || !notices) { free(launcher); free(notices); return 1; }
    if (malvado_export_status() != 2) {
        fprintf(stderr, "Export requires an installed SDK with game launchers and licenses\n");
        free(launcher); free(notices); return 1;
    }
    if (!copy_tree(".", NULL, 0, native)) {
        free(launcher); free(notices); return 1;
    }
    if (mkdir(output, 0755) != 0) { perror(output); free(launcher); free(notices); return 1; }
    char *contents = app ? join_path(output, "Contents") : NULL;
    char *payload = app && contents ? join_path(contents, "MacOS") : (app ? NULL : strdup(output));
    char *plist = app && contents ? join_path(contents, "Info.plist") : NULL;
    int ok = payload && (!app || (contents && plist && mkdir(contents, 0755) == 0 &&
                                  mkdir(payload, 0755) == 0 && write_bundle_info(plist, app_name, app_name_length)));
    char *game = payload ? join_path(payload, "game") : NULL;
    char *license_dir = payload ? join_path(payload, "licenses") : NULL;
    char *native_game = native && payload ? join_path(payload, "game-bin") : NULL;
    ok = ok && game && license_dir && (!native || native_game) && copy_file(launcher, game, 1) &&
             (!native || copy_file(".malvado/build/game", native_game, 1)) &&
             mkdir(license_dir, 0755) == 0 && copy_tree(notices, license_dir, 1, 0) &&
             copy_tree(".", payload, 0, native);
    free(native_game); free(game); free(license_dir); free(plist); free(payload); free(contents);
    free(launcher); free(notices);
    if (!ok) { fprintf(stderr, "Export incomplete; remove the new directory: %s\n", output); return 1; }
    printf("Exported game to %s\n", output);
    return 0;
}

int
malvado_export_default(int native, int app)
{
    struct stat info;
    const char *entry = native ? "main.c" : "main.lua";
    if (lstat(entry, &info) != 0 || !S_ISREG(info.st_mode)) {
        fprintf(stderr, "Project needs a regular %s file\n", entry);
        return 1;
    }
    if (malvado_export_status() != 2) {
        fprintf(stderr, "Export requires an installed SDK with game launchers and licenses\n");
        return 1;
    }
    if (lstat("dist", &info) != 0) {
        if (errno != ENOENT || mkdir("dist", 0755) != 0) {
            perror("dist");
            return 1;
        }
    } else if (!S_ISDIR(info.st_mode)) {
        fprintf(stderr, "dist must be a directory, not a link or file\n");
        return 1;
    }
    return malvado_export(app ? "dist/game.app" : "dist/game", native, app);
}
#else
#include <stdio.h>
#include "export.h"
int
malvado_sdk_root(char *path, size_t size)
{
    (void)path;
    (void)size;
    return 0;
}
int
malvado_export_status(void)
{
    return 0;
}
int
malvado_export(const char *output, int native, int app)
{
    (void)output;
    (void)native;
    (void)app;
    fprintf(stderr, "Game export requires a graphical macOS or Linux SDK\n");
    return 1;
}
int
malvado_export_default(int native, int app)
{
    return malvado_export(NULL, native, app);
}
#endif
