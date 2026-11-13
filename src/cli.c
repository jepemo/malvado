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

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define mkdir_project(path) _mkdir(path)
#define remove_project(path) _rmdir(path)
#define change_directory(path) _chdir(path)
#else
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#define mkdir_project(path) mkdir(path, 0777)
#define remove_project(path) rmdir(path)
#define change_directory(path) chdir(path)
#endif
#include "lua.h"
#include "lauxlib.h"
#include "runner/runtime.h"
#include "build.h"
#include "export.h"
#ifdef MALVADO_BUILD_ASSETS
#include "fpg_export.h"
#endif

#ifndef MALVADO_VERSION_STRING
#define MALVADO_VERSION_STRING "development"
#endif

static void
print_help(FILE *output)
{
    fprintf(output, "Usage: malvado COMMAND [OPTIONS]\n\n"
            "  new [--c | --lang lua|c|rust] PATH  Create a project\n"
            "  build [PATH]         Build a C or Rust project\n"
            "  run [--headless] [PATH]  Run a game\n"
            "  check [PATH]         Check Lua syntax or a C/Rust project\n"
            "  export [PATH]        Export a game for this host\n"
            "  export --app [--id ID] [PATH]  Export a macOS app\n"
#ifdef MALVADO_BUILD_ASSETS
            "  fpg-export [--transparent-zero] INPUT OUTPUT  Export FPG to PNG and JSON\n"
#endif
            "  doctor               Show runtime capabilities\n"
            "  --version            Show the version\n");
}

static int
tool_available(const char *tool)
{
#ifdef _WIN32
    return SearchPathA(NULL, tool, ".exe", 0, NULL, NULL) != 0;
#else
    const char *path = getenv("PATH");
    if (!path) return 0;
    const char *start = path;
    for (;;) {
        const char *end = strchr(start, ':');
        size_t length = end ? (size_t)(end - start) : strlen(start);
        char *candidate = malloc(length + strlen(tool) + 3);
        if (!candidate) return 0;
        if (length) {
            memcpy(candidate, start, length);
            candidate[length] = '/';
            strcpy(candidate + length + 1, tool);
        } else snprintf(candidate, strlen(tool) + 3, "./%s", tool);
        struct stat info;
        int found = stat(candidate, &info) == 0 && S_ISREG(info.st_mode) && access(candidate, X_OK) == 0;
        free(candidate);
        if (found) return 1;
        if (!end) return 0;
        start = end + 1;
    }
#endif
}

static int
check_script(const char *path)
{
    lua_State *L = luaL_newstate();
    if (!L) {
        fprintf(stderr, "Cannot create Lua state\n");
        return 1;
    }
    int result = luaL_loadfilex(L, path, "t");
    if (result != LUA_OK)
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
    else
        printf("Syntax OK: %s\n", path);
    lua_close(L);
    return result == LUA_OK ? 0 : 1;
}

static int check_directory(const char *path, unsigned depth, unsigned *files, int for_export);

static int
check_entry(const char *directory, const char *name, int is_directory,
            unsigned depth, unsigned *files, int for_export)
{
    if (name[0] == '.')
        return 0;
    if (for_export && is_directory && (!strcmp(name, "build") || !strcmp(name, "dist")))
        return 0;
    size_t length = strlen(directory) + strlen(name) + 2;
    char *path = malloc(length);
    if (!path) {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }
    snprintf(path, length, "%s/%s", directory, name);
    int result = 0;
    if (is_directory) {
        result = check_directory(path, depth + 1, files, for_export);
    } else {
        size_t name_length = strlen(name);
        if (name_length >= 4 && strcmp(name + name_length - 4, ".lua") == 0) {
            ++*files;
            result = check_script(path);
        }
    }
    free(path);
    return result;
}

static int
check_directory(const char *path, unsigned depth, unsigned *files, int for_export)
{
    if (depth > 64) {
        fprintf(stderr, "Directory nesting exceeds 64: %s\n", path);
        return 1;
    }
    int failed = 0;
#ifdef _WIN32
    size_t length = strlen(path) + 3;
    char *pattern = malloc(length);
    if (!pattern) return 1;
    snprintf(pattern, length, "%s/*", path);
    WIN32_FIND_DATAA entry;
    HANDLE handle = FindFirstFileA(pattern, &entry);
    free(pattern);
    if (handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Cannot read directory: %s\n", path);
        return 1;
    }
    do {
        if (!(entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
            failed |= check_entry(path, entry.cFileName,
                (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0, depth, files, for_export);
    } while (FindNextFileA(handle, &entry));
    if (GetLastError() != ERROR_NO_MORE_FILES) failed = 1;
    FindClose(handle);
#else
    DIR *directory = opendir(path);
    if (!directory) {
        perror(path);
        return 1;
    }
    struct dirent *entry;
    for (;;) {
        errno = 0;
        entry = readdir(directory);
        if (!entry) {
            if (errno != 0) { perror(path); failed = 1; }
            break;
        }
        if (entry->d_name[0] == '.') continue;
        size_t length = strlen(path) + strlen(entry->d_name) + 2;
        char *full = malloc(length);
        if (!full) { failed = 1; break; }
        snprintf(full, length, "%s/%s", path, entry->d_name);
        struct stat info;
        if (lstat(full, &info) != 0) {
            perror(full);
            failed = 1;
        } else if (S_ISREG(info.st_mode) || S_ISDIR(info.st_mode)) {
            failed |= check_entry(path, entry->d_name, S_ISDIR(info.st_mode), depth, files, for_export);
        }
        free(full);
    }
    closedir(directory);
#endif
    return failed;
}

static int
is_directory(const char *path)
{
#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat info;
    return stat(path, &info) == 0 && S_ISDIR(info.st_mode);
#endif
}

static int
is_regular_file(const char *path)
{
#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    struct stat info;
    return stat(path, &info) == 0 && S_ISREG(info.st_mode);
#endif
}

/* Preserve historical Lua-over-C precedence. Rust must be unambiguous. */
static int
project_language(void)
{
    int lua = is_regular_file("main.lua");
    int native = malvado_native_project();
    if (is_regular_file("Cargo.toml")) {
        if (lua || native) {
            fprintf(stderr, "Ambiguous project: Cargo.toml cannot coexist with a Lua or C entry\n");
            return -1;
        }
        return 2;
    }
    return lua ? 0 : native ? 1 : 0;
}

static int
check_project(const char *path, int for_export)
{
    unsigned files = 0;
    int failed = check_directory(path, 0, &files, for_export);
    if (!files) {
        fprintf(stderr, "No Lua files found\n");
        return 1;
    }
    return failed ? 1 : 0;
}

static int
run_game(char *program, char *path, int headless, int detect_project)
{
    if (is_directory(path)) {
        if (change_directory(path) != 0) {
            perror(path);
            return 1;
        }
        path = "main.lua";
        detect_project = 1;
    }
    int language = detect_project ? project_language() : 0;
    if (language < 0) return 2;
    if (language == 2) {
        if (headless) {
            fprintf(stderr, "--headless is only available for Lua projects; Rust owns its host loop\n");
            return 2;
        }
        return malvado_rust_run();
    }
    if (language == 1) {
        if (headless) {
            fprintf(stderr, "Headless mode is not available for C projects\n");
            return 2;
        }
        return malvado_native_run();
    }
    if (headless) {
        char *args[] = {program, "--headless", path};
        return malvado_run(3, args);
    }
    char *args[] = {program, path};
    return malvado_run(2, args);
}

static int
write_project_file(const char *path, const char *name, const char *content)
{
    size_t size = strlen(path) + strlen(name) + 2;
    char *file_path = malloc(size);
    if (!file_path) return 0;
    snprintf(file_path, size, "%s/%s", path, name);
    FILE *file = fopen(file_path, "wx");
    if (!file) {
        perror(file_path);
        free(file_path);
        return 0;
    }
    size_t length = strlen(content);
    int failed = fwrite(content, 1, length, file) != length;
    if (fclose(file) != 0) failed = 1;
    if (failed) {
        fprintf(stderr, "Cannot write %s\n", file_path);
        remove(file_path);
    }
    free(file_path);
    return !failed;
}

static void
remove_project_file(const char *path, const char *name)
{
    size_t size = strlen(path) + strlen(name) + 2;
    char *file_path = malloc(size);
    if (!file_path) return;
    snprintf(file_path, size, "%s/%s", path, name);
    remove(file_path);
    free(file_path);
}

static char *
rust_manifest(void)
{
    char dependency[4096];
    if (!malvado_rust_dependency(dependency, sizeof(dependency))) {
        fprintf(stderr, "Rust projects require an installed SDK with Rust crate sources\n");
        return NULL;
    }
    char escaped[sizeof(dependency) * 2];
    size_t used = 0;
    for (const unsigned char *p = (const unsigned char *)dependency; *p; ++p) {
        if (*p < 32 || *p == 127 || *p == '"' || *p == ';') {
            fprintf(stderr, "Unsupported character in SDK path (control character, quotation mark or semicolon)\n");
            return NULL;
        }
        if (*p == '\\') escaped[used++] = '\\';
        escaped[used++] = (char)*p;
    }
    escaped[used] = '\0';
    const char format[] = "[package]\nname = \"game\"\nversion = \"0.1.0\"\nedition = \"2024\"\n"
        "\n[[bin]]\nname = \"game\"\npath = \"main.rs\"\n\n[dependencies]\nmalvado = { path = \"%s\", features = [\"graphics\"] }\n";
    size_t size = sizeof(format) + used;
    char *manifest = malloc(size);
    if (manifest) snprintf(manifest, size, format, escaped);
    return manifest;
}

static int
new_project(const char *path, int language)
{
    static const char game[] =
        "require(\"malvado\").import_globals()\n\n"
        "local Game = process(function()\n"
        "  while not key(\"escape\") do\n"
        "    draw_text(\"Hello, Malvado!\", 40, 40, 30)\n"
        "    frame()\n"
        "  end\n"
        "end)\n\n"
        "return {\n"
        "  title = \"My game\",\n"
        "  init = function() Game() end,\n"
        "}\n";
    static const char native_game[] =
        "#include <malvado/platform.h>\n\n"
        "int main(void)\n"
        "{\n"
        "    if (!mv_window_open(800, 600, \"Hello world\")) return 1;\n"
        "    while (!mv_window_should_close() && !mv_window_escape_down()) {\n"
        "        mv_window_begin((MvColor){25, 30, 45, 255});\n"
        "        mv_window_draw_text(\"Hello, Malvado!\", 40, 40, 30, (MvColor){255, 255, 255, 255});\n"
        "        mv_window_end();\n"
        "    }\n"
        "    mv_window_close();\n"
        "    return 0;\n"
        "}\n";
    static const char native_cmake[] =
        "cmake_minimum_required(VERSION 3.20)\n"
        "project(game C)\n"
        "set(CMAKE_C_STANDARD 11)\n"
        "find_package(Malvado CONFIG REQUIRED COMPONENTS graphics)\n"
        "add_executable(game main.c)\n"
        "target_link_libraries(game PRIVATE Malvado::graphics)\n";
    static const char rust_game[] =
        "use malvado::{Game, Error, SpawnOptions};\n"
        "use malvado::window::{Color, Key, Window};\n\n"
        "fn main() -> Result<(), Error> {\n"
        "    let window = Window::open(800, 600, \"Hello, Malvado!\")?;\n"
        "    let mut game = Game::new()?;\n"
        "    game.spawn(SpawnOptions::default(), |p| async move {\n"
        "        loop {\n"
        "            p.current().draw_text(\"Hello, Malvado!\", (40, 40), 30, Color { r: 255, g: 255, b: 255, a: 255 })?;\n"
        "            p.frame().await;\n"
        "        }\n"
        "    })?;\n"
        "    let mut previous = std::time::Instant::now();\n"
        "    while !window.should_close() && !window.key_down(Key::Escape) {\n"
        "        let now = std::time::Instant::now();\n"
        "        for _ in 0..game.advance_time((now - previous).as_secs_f64())? {\n"
        "            window.input_tick();\n"
        "            game.tick()?;\n"
        "        }\n"
        "        previous = now;\n"
        "        let frame = window.frame(Color { r: 25, g: 30, b: 45, a: 255 })?;\n"
        "        frame.process_graphics(&game)?;\n"
        "    }\n"
        "    Ok(())\n"
        "}\n";
    size_t length = strlen(path);
    if (!length || path[0] == '-' || length > 4096) {
        fprintf(stderr, "Invalid project path\n");
        return 2;
    }
    char *manifest = language == 2 ? rust_manifest() : NULL;
    if (language == 2 && !manifest) return 1;
    if (mkdir_project(path) != 0) {
        perror("Cannot create project directory");
        free(manifest);
        return 1;
    }
    if (language == 2) {
        if (!write_project_file(path, "main.rs", rust_game)) {
            free(manifest);
            remove_project(path);
            return 1;
        }
        int ok = write_project_file(path, "Cargo.toml", manifest);
        free(manifest);
        if (!ok) {
            remove_project_file(path, "main.rs");
            remove_project(path);
            return 1;
        }
        printf("Created %s. Enter the directory and run: malvado run\n", path);
        return 0;
    }
    int native = language == 1;
    const char *main_file = native ? "main.c" : "main.lua";
    if (!write_project_file(path, main_file, native ? native_game : game)) {
        remove_project(path);
        return 1;
    }
    if (native && !write_project_file(path, "CMakeLists.txt", native_cmake)) {
        remove_project_file(path, main_file);
        remove_project(path);
        return 1;
    }
    printf("Created %s. %s\n", path, native ?
           "Configure with CMake and the Malvado SDK." :
           "Enter the project directory and run: malvado run");
    return 0;
}

int
main(int argc, char **argv)
{
    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h") || !strcmp(argv[1], "help"))) {
        print_help(stdout);
        return 0;
    }
    if (argc == 2 && !strcmp(argv[1], "--version")) {
        printf("malvado %s\n", MALVADO_VERSION_STRING);
        return 0;
    }
#ifdef MALVADO_BUILD_ASSETS
    if (argc >= 2 && strcmp(argv[1], "fpg-export") == 0) {
        int transparent = argc >= 3 && !strcmp(argv[2], "--transparent-zero");
        int first = transparent ? 3 : 2;
        if (argc != first + 2 || argv[first][0] == '-' || argv[first + 1][0] == '-') {
            print_help(stderr);
            return 2;
        }
        return malvado_fpg_export(argv[first], argv[first + 1], transparent);
    }
#endif
    if (argc == 2 && strcmp(argv[1], "doctor") == 0) {
        printf("Lua: %s\n", LUA_RELEASE);
        printf("CMake: %s\n", tool_available("cmake") ? "found" : "not found");
        printf("Cargo: %s\n", tool_available("cargo") ? "found" : "not found");
        printf("Rust compiler: %s\n", tool_available("rustc") ? "found" : "not found");
        char dependency[4096];
        printf("Rust SDK: %s\n", malvado_rust_dependency(dependency, sizeof(dependency)) ?
               dependency : "not found; install a graphical SDK with Rust crate sources");
#ifdef MALVADO_GRAPHICS_ENABLED
        printf("Graphics: enabled\n");
#else
        printf("Graphics: headless\n");
#endif
        int status = malvado_export_status();
        if (status == 2)
            printf("Export: launchers and required licenses found\n");
        else if (status == 1)
            printf("Export: launchers or required licenses missing; install the SDK\n");
        else
            printf("Export: unavailable in this build\n");
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "new") == 0)
        return new_project(argv[2], 0);
    if (argc == 4 && strcmp(argv[1], "new") == 0 && strcmp(argv[2], "--c") == 0)
        return new_project(argv[3], 1);
    if (argc == 5 && !strcmp(argv[1], "new") && !strcmp(argv[2], "--lang")) {
        int language = !strcmp(argv[3], "lua") ? 0 : !strcmp(argv[3], "c") ? 1 :
                       !strcmp(argv[3], "rust") ? 2 : -1;
        if (language < 0) { print_help(stderr); return 2; }
        return new_project(argv[4], language);
    }
    if (argc >= 2 && argc <= 3 && strcmp(argv[1], "build") == 0) {
        if (argc == 3 && change_directory(argv[2]) != 0) {
            perror(argv[2]);
            return 1;
        }
        int language = project_language();
        if (language < 0) return 2;
        return language == 2 ? malvado_rust_build(0) : malvado_native_build();
    }
    if (argc >= 2 && strcmp(argv[1], "export") == 0) {
        int app = argc >= 3 && strcmp(argv[2], "--app") == 0;
        const char *bundle_id = NULL, *destination = NULL;
        for (int i = app ? 3 : 2; i < argc; ++i) {
            if (app && strcmp(argv[i], "--id") == 0) {
                if (bundle_id || ++i >= argc) { print_help(stderr); return 2; }
                bundle_id = argv[i];
            } else if (destination || argv[i][0] == '-') {
                print_help(stderr); return 2;
            } else destination = argv[i];
        }
#ifndef __APPLE__
        if (app) { fprintf(stderr, "App export requires macOS\n"); return 1; }
#endif
        int native = project_language();
        if (native < 0) return 2;
        int built = native == 2 ? malvado_rust_build(0) :
                    native == 1 ? malvado_native_build() : check_project(".", 1);
        if (built != 0) return 1;
        return destination ? malvado_export(destination, native, app, bundle_id) :
                             malvado_export_default(native, app, bundle_id);
    }
    if (argc >= 2 && argc <= 3 && strcmp(argv[1], "check") == 0) {
        if (argc == 3 && !is_directory(argv[2])) return check_script(argv[2]);
        if (argc == 3 && change_directory(argv[2]) != 0) { perror(argv[2]); return 1; }
        int language = project_language();
        if (language < 0) return 2;
        return language == 2 ? malvado_rust_build(1) :
               language == 1 ? malvado_native_build() : check_project(".", 0);
    }
    if (argc >= 2 && strcmp(argv[1], "run") == 0) {
        if (argc == 2) {
            return run_game(argv[0], "main.lua", 0, 1);
        }
        if (argc == 3) {
            if (strcmp(argv[2], "--headless") == 0) {
                return run_game(argv[0], "main.lua", 1, 1);
            }
            return run_game(argv[0], argv[2], 0, 0);
        }
        if (argc == 4 && strcmp(argv[2], "--headless") == 0) {
            return run_game(argv[0], argv[3], 1, 0);
        }
    }
    print_help(stderr);
    return 2;
}
