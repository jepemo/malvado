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
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "build.h"
#include "export.h"

static const char marker_text[] = "malvado-c-build-v1\n";

static int
regular_file(const char *path)
{
    struct stat info;
    return lstat(path, &info) == 0 && S_ISREG(info.st_mode);
}

int
malvado_native_project(void)
{
    return regular_file("main.c") && regular_file("CMakeLists.txt");
}

static int
ensure_directory(const char *path, int *created)
{
    struct stat info;
    *created = 0;
    if (lstat(path, &info) == 0) {
        if (S_ISDIR(info.st_mode)) return 1;
        fprintf(stderr, "%s must be a directory, not a link or file\n", path);
        return 0;
    }
    if (errno != ENOENT || mkdir(path, 0755) != 0) {
        perror(path);
        return 0;
    }
    *created = 1;
    return 1;
}

static int
prepare_build(void)
{
    int created;
    if (!ensure_directory(".malvado", &created)) return 0;
    if (!ensure_directory(".malvado/build", &created)) return 0;
    const char *marker = ".malvado/build/.malvado-owner";
    if (created) {
        FILE *file = fopen(marker, "wx");
        if (!file) { perror(marker); return 0; }
        int ok = fwrite(marker_text, 1, sizeof(marker_text) - 1, file) == sizeof(marker_text) - 1;
        if (fclose(file) != 0) ok = 0;
        if (!ok) fprintf(stderr, "Cannot write build marker\n");
        return ok;
    }
    if (!regular_file(marker)) {
        fprintf(stderr, "Existing .malvado/build is not managed by Malvado\n");
        return 0;
    }
    FILE *file = fopen(marker, "rb");
    if (!file) { perror(marker); return 0; }
    char content[sizeof(marker_text)];
    size_t length = fread(content, 1, sizeof(content), file);
    int ok = !ferror(file) && length == sizeof(marker_text) - 1 &&
             memcmp(content, marker_text, sizeof(marker_text) - 1) == 0;
    if (fclose(file) != 0) ok = 0;
    if (!ok) fprintf(stderr, "Invalid Malvado build marker\n");
    return ok;
}

static int
run_process(char *const args[])
{
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        execvp(args[0], args);
        perror(args[0]);
        _exit(127);
    }
    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) { perror("waitpid"); return 1; }
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int
malvado_native_build(void)
{
    if (!malvado_native_project()) {
        fprintf(stderr, "C project needs main.c and CMakeLists.txt\n");
        return 1;
    }
    char sdk[PATH_MAX];
    if (malvado_export_status() != 2 || !malvado_sdk_root(sdk, sizeof(sdk))) {
        fprintf(stderr, "C builds require an installed graphical SDK\n");
        return 1;
    }
    if (!prepare_build()) return 1;
    size_t size = strlen(sdk) + sizeof("-DCMAKE_PREFIX_PATH=");
    char *prefix = malloc(size);
    if (!prefix) return 1;
    snprintf(prefix, size, "-DCMAKE_PREFIX_PATH=%s", sdk);
    char *configure[] = {"cmake", "-S", ".", "-B", ".malvado/build", prefix, "-DCMAKE_BUILD_TYPE=Debug", NULL};
    int result = run_process(configure);
    free(prefix);
    if (result != 0) return 1;
    char *build[] = {"cmake", "--build", ".malvado/build", "--config", "Debug", "--target", "game", NULL};
    return run_process(build) == 0 ? 0 : 1;
}

int
malvado_native_run(void)
{
    if (malvado_native_build() != 0) return 1;
    char *run[] = {".malvado/build/game", NULL};
    return run_process(run);
}
#else
#include <stdio.h>
#include "build.h"
int malvado_native_project(void) { return 0; }
int malvado_native_build(void) { fprintf(stderr, "C builds require a graphical macOS or Linux SDK\n"); return 1; }
int malvado_native_run(void) { return malvado_native_build(); }
#endif
