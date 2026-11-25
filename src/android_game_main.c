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

#ifdef MALVADO_ANDROID_NATIVE_GAME
#undef main
#endif
#define _XOPEN_SOURCE 700
#include <android/asset_manager.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef MALVADO_ANDROID_NATIVE_GAME
extern int malvado_game_main(void);
#else
#include "runner/runtime.h"
#endif

extern struct android_app *GetAndroidApp(void);

static int
remove_entry(const char *path, const struct stat *info, int kind, struct FTW *position)
{
    (void)info;
    (void)kind;
    (void)position;
    return remove(path);
}

static int
copy_asset(AAssetManager *manager, char *name)
{
    /* Only ordinary relative components are accepted. The exporter applies
     * the same contract before publishing an APK. */
    if (!*name || *name == '/' || strchr(name, '\\') || strchr(name, '\r')) return 0;
    for (char *part = name; part;) {
        char *slash = strchr(part, '/');
        size_t length = slash ? (size_t)(slash - part) : strlen(part);
        if (!length || (length == 1 && *part == '.') ||
            (length == 2 && !memcmp(part, "..", 2))) return 0;
        if (slash) {
            *slash = 0;
            int result = mkdir(name, 0700);
            *slash = '/';
            if (result != 0) {
                struct stat info;
                *slash = 0;
                int directory = lstat(name, &info) == 0 && S_ISDIR(info.st_mode);
                *slash = '/';
                if (!directory) return 0;
            }
        }
        part = slash ? slash + 1 : NULL;
    }
    AAsset *asset = AAssetManager_open(manager, name, AASSET_MODE_STREAMING);
    if (!asset) return 0;
    int fd = open(name, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    int ok = fd >= 0;
    char bytes[16384];
    int count;
    while (ok && (count = AAsset_read(asset, bytes, sizeof(bytes))) > 0) {
        int offset = 0;
        while (offset < count) {
            ssize_t written = write(fd, bytes + offset, (size_t)(count - offset));
            if (written <= 0) {
                ok = 0;
                break;
            }
            offset += (int)written;
        }
    }
    if (ok && count < 0) ok = 0;
    if (fd >= 0 && close(fd) != 0) ok = 0;
    AAsset_close(asset);
    return ok;
}

static int
stage_game(AAssetManager *manager)
{
    AAsset *index = AAssetManager_open(manager, "malvado-files.txt", AASSET_MODE_STREAMING);
    if (!index) return 0;
    char name[1024];
    size_t length = 0;
    int ok = 1, files = 0;
    char byte;
    int count;
    while (ok && (count = AAsset_read(index, &byte, 1)) == 1) {
        if (byte == '\n') {
            name[length] = 0;
            ok = copy_asset(manager, name);
            length = 0;
            files++;
        } else if (!byte || length == sizeof(name) - 1) ok = 0;
        else name[length++] = byte;
    }
    AAsset_close(index);
    return ok && count == 0 && length == 0 && files > 0;
}

static int
run_game(void)
{
    char diagnostic[] = ".malvado-stderr-XXXXXX";
    int saved = dup(STDERR_FILENO);
    int file = mkstemp(diagnostic);
    if (saved < 0 || file < 0) {
        if (saved >= 0) close(saved);
        if (file >= 0) close(file);
        return 1;
    }
    fflush(stderr);
    if (dup2(file, STDERR_FILENO) < 0) {
        close(saved);
        close(file);
        return 1;
    }
#ifdef MALVADO_ANDROID_NATIVE_GAME
    char *directory = getcwd(NULL, 0);
    const char *inherited = getenv("MALVADO_GAME_DIR");
    char *previous = inherited ? strdup(inherited) : NULL;
    int result = 1;
    if (directory && (!inherited || previous) && setenv("MALVADO_GAME_DIR", directory, 1) == 0) {
        result = malvado_game_main();
        if (previous ? setenv("MALVADO_GAME_DIR", previous, 1) : unsetenv("MALVADO_GAME_DIR"))
            result = 1;
    }
    free(previous);
    free(directory);
#else
    char *arguments[] = {"game", "main.lua"};
    int result = malvado_run(2, arguments);
#endif
    fflush(stderr);
    if (dup2(saved, STDERR_FILENO) < 0) result = 1;
    close(saved);
    if (lseek(file, 0, SEEK_SET) >= 0) {
        char text[1024];
        ssize_t length;
        while ((length = read(file, text, sizeof(text) - 1)) > 0) {
            text[length] = 0;
            __android_log_write(ANDROID_LOG_ERROR, "MalvadoGame", text);
        }
    }
    close(file);
    return result;
}

int
main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    struct android_app *app = GetAndroidApp();
    if (!app || !app->activity) {
        __android_log_print(ANDROID_LOG_ERROR, "MalvadoGame", "NativeActivity is unavailable");
        return 1;
    }
    char directory[4096];
    if (snprintf(directory, sizeof(directory), "%s/malvado-game-XXXXXX",
                 app->activity->internalDataPath) >= (int)sizeof(directory)) return 1;
    if (chdir(app->activity->internalDataPath) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, "MalvadoGame", "Cannot enter application storage: %s", strerror(errno));
        return 1;
    }
    if (!mkdtemp(directory)) {
        __android_log_print(ANDROID_LOG_ERROR, "MalvadoGame", "Cannot create game directory: %s", strerror(errno));
        return 1;
    }
    int result = 1;
    if (chdir(directory) == 0 && stage_game(app->activity->assetManager)) {
        result = run_game();
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "MalvadoGame", "Cannot prepare packaged game assets");
    }
    if (chdir(app->activity->internalDataPath) != 0) result = 1;
    int cleaned = nftw(directory, remove_entry, 16, FTW_DEPTH | FTW_PHYS) == 0;
    if (!cleaned) result = 1;
    __android_log_print(ANDROID_LOG_INFO, "MalvadoGame", "game session cleanup %s", cleaned ? "passed" : "failed");
    __android_log_print(ANDROID_LOG_INFO, "MalvadoGame", "game exited with status %d", result);
    return result;
}
