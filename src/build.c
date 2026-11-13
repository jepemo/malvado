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

int
malvado_rust_dependency(char *path, size_t size)
{
    char sdk[PATH_MAX];
    if (!malvado_sdk_root(sdk, sizeof(sdk))) return 0;
    int length = snprintf(path, size, "%s/%s", sdk, MALVADO_RUST_CRATE_REL);
    if (length < 0 || (size_t)length >= size) return 0;
    char manifest[PATH_MAX];
    length = snprintf(manifest, sizeof(manifest), "%s/Cargo.toml", path);
    return length >= 0 && (size_t)length < sizeof(manifest) && regular_file(manifest);
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
prepare_build(int rust)
{
    int created;
    if (!ensure_directory(".malvado", &created)) return 0;
    if (!ensure_directory(rust ? ".malvado/rust" : ".malvado/build", &created)) return 0;
    const char *marker = rust ? ".malvado/rust/.malvado-owner" : ".malvado/build/.malvado-owner";
    const char *text = rust ? "malvado-rust-build-v1\n" : marker_text;
    size_t text_length = strlen(text);
    if (created) {
        FILE *file = fopen(marker, "wx");
        if (!file) { perror(marker); return 0; }
        int ok = fwrite(text, 1, text_length, file) == text_length;
        if (fclose(file) != 0) ok = 0;
        if (!ok) fprintf(stderr, "Cannot write build marker\n");
        return ok;
    }
    if (!regular_file(marker)) {
        fprintf(stderr, "Existing build directory is not managed by Malvado\n");
        return 0;
    }
    FILE *file = fopen(marker, "rb");
    if (!file) { perror(marker); return 0; }
    char content[64];
    size_t length = fread(content, 1, sizeof(content), file);
    int ok = !ferror(file) && length == text_length &&
             memcmp(content, text, text_length) == 0;
    if (fclose(file) != 0) ok = 0;
    if (!ok) fprintf(stderr, "Invalid Malvado build marker\n");
    return ok;
}

static int
run_process_with_sdk(char *const args[], const char *sdk)
{
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        if (setenv("RUSTUP_AUTO_INSTALL", "0", 1) != 0 ||
            (sdk && setenv("MALVADO_SDK_PREFIX", sdk, 1) != 0)) {
            perror("setenv");
            _exit(1);
        }
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

static int
run_process(char *const args[])
{
    return run_process_with_sdk(args, NULL);
}

/* Select the host explicitly so Cargo configuration cannot silently redirect
 * exports to a different target or leave a stale default-target executable. */
static int
rust_host(char *host, size_t size)
{
    int channel[2];
    if (pipe(channel) != 0) { perror("pipe"); return 0; }
    pid_t pid = fork();
    if (pid < 0) { close(channel[0]); close(channel[1]); perror("fork"); return 0; }
    if (pid == 0) {
        close(channel[0]);
        if (dup2(channel[1], STDOUT_FILENO) < 0 || setenv("RUSTUP_AUTO_INSTALL", "0", 1) != 0)
            _exit(1);
        close(channel[1]);
        execlp("rustc", "rustc", "-vV", (char *)NULL);
        perror("rustc (install a Rust toolchain before building Rust projects)");
        _exit(127);
    }
    close(channel[1]);
    char output[4096];
    size_t used = 0;
    int ok = 1;
    for (;;) {
        char buffer[512];
        ssize_t count = read(channel[0], buffer, sizeof(buffer));
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) { if (count < 0) ok = 0; break; }
        if ((size_t)count >= sizeof(output) - used) ok = 0;
        else if (ok) { memcpy(output + used, buffer, (size_t)count); used += (size_t)count; }
    }
    close(channel[0]);
    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) { perror("waitpid"); return 0; }
    }
    if (!ok || !WIFEXITED(status) || WEXITSTATUS(status) != 0) return 0;
    output[used] = '\0';
    char *start = strstr(output, "\nhost: ");
    if (!start) return 0;
    start += strlen("\nhost: ");
    size_t length = strcspn(start, "\r\n");
    if (!length || length >= size) return 0;
    for (size_t i = 0; i < length; ++i) {
        unsigned char c = (unsigned char)start[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_')) return 0;
    }
    memcpy(host, start, length);
    host[length] = '\0';
    return 1;
}

int
malvado_rust_binary(char *path, size_t size)
{
    char host[128];
    if (!rust_host(host, sizeof(host))) return 0;
    int length = snprintf(path, size, ".malvado/rust/%s/debug/game", host);
    return length >= 0 && (size_t)length < size && regular_file(path);
}

int
malvado_rust_build(int check)
{
    char sdk[PATH_MAX], dependency[PATH_MAX], host[128];
    if (!regular_file("Cargo.toml")) {
        fprintf(stderr, "Rust project needs Cargo.toml and a binary target named game\n");
        return 1;
    }
    if (!malvado_sdk_root(sdk, sizeof(sdk)) ||
        !malvado_rust_dependency(dependency, sizeof(dependency)) || malvado_export_status() != 2) {
        fprintf(stderr, "Rust builds require an installed graphical SDK with Rust crate sources\n");
        return 1;
    }
    if (!rust_host(host, sizeof(host)) || !prepare_build(1)) return 1;
    char *args[] = {"cargo", check ? "check" : "build", "--bin", "game", "--target", host,
                    "--target-dir", ".malvado/rust", NULL};
    return run_process_with_sdk(args, sdk) == 0 ? 0 : 1;
}

int
malvado_rust_run(void)
{
    if (malvado_rust_build(0) != 0) return 1;
    char binary[PATH_MAX];
    if (!malvado_rust_binary(binary, sizeof(binary))) {
        fprintf(stderr, "Cargo did not produce the game executable\n");
        return 1;
    }
    char *args[] = {binary, NULL};
    return run_process(args);
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
    if (!prepare_build(0)) return 1;
    size_t size = strlen(sdk) + sizeof("-DCMAKE_PREFIX_PATH=");
    char *prefix = malloc(size);
    if (!prefix) return 1;
    snprintf(prefix, size, "-DCMAKE_PREFIX_PATH=%s", sdk);
    char current[PATH_MAX];
    if (!getcwd(current, sizeof(current))) { perror("getcwd"); free(prefix); return 1; }
    size_t output_size = strlen(current) + sizeof("-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=/.malvado/build");
    char *output = malloc(output_size);
    if (!output) { free(prefix); return 1; }
    snprintf(output, output_size, "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=%s/.malvado/build", current);
    char *configure[] = {"cmake", "-S", ".", "-B", ".malvado/build", prefix,
                         "-DCMAKE_BUILD_TYPE=Debug", output, NULL};
    int result = run_process(configure);
    free(prefix);
    free(output);
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
int malvado_rust_dependency(char *path, size_t size) { (void)path; (void)size; return 0; }
int malvado_rust_binary(char *path, size_t size) { (void)path; (void)size; return 0; }
int malvado_rust_build(int check) { (void)check; fprintf(stderr, "Rust builds require a graphical macOS or Linux SDK\n"); return 1; }
int malvado_rust_run(void) { return malvado_rust_build(0); }
int malvado_native_project(void) { return 0; }
int malvado_native_build(void) { fprintf(stderr, "C builds require a graphical macOS or Linux SDK\n"); return 1; }
int malvado_native_run(void) { return malvado_native_build(); }
#endif
