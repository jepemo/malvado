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
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "windows_process.h"

static int
append(char **cursor, char *end, char value)
{
    if (*cursor == end) return 0;
    *(*cursor)++ = value;
    return 1;
}

/* Encode the Windows CRT argv rules, including empty values, embedded quotes
 * and backslashes before quotes or the closing delimiter. No shell is used. */
static char *
command_line(char *const args[])
{
    char *line = malloc(32768);
    if (!line) return NULL;
    char *cursor = line, *end = line + 32767;
    for (size_t i = 0; args[i]; ++i) {
        if (i && !append(&cursor, end, ' ')) goto fail;
        if (!append(&cursor, end, '"')) goto fail;
        const char *text = args[i];
        for (;;) {
            size_t slashes = 0;
            while (*text == '\\') { ++slashes; ++text; }
            size_t count = (*text == '"' || !*text) ? slashes * 2 : slashes;
            if (*text == '"') ++count;
            for (size_t j = 0; j < count; ++j)
                if (!append(&cursor, end, '\\')) goto fail;
            if (!*text) break;
            if (!append(&cursor, end, *text++)) goto fail;
        }
        if (!append(&cursor, end, '"')) goto fail;
    }
    *cursor = '\0';
    return line;
fail:
    free(line);
    fprintf(stderr, "Child command exceeds Windows command-line limit\n");
    return NULL;
}

static int
compare_environment(const void *left, const void *right)
{
    return _stricmp(*(const char *const *)left, *(const char *const *)right);
}

static char *
child_environment(const char *sdk)
{
    char *original = GetEnvironmentStringsA();
    if (!original) return NULL;
    size_t count = 0;
    for (const char *p = original; *p; p += strlen(p) + 1) ++count;
    const char **entries = calloc(count + 2, sizeof(*entries));
    char *sdk_entry = NULL, *block = NULL;
    if (!entries) goto done;
    size_t used = 0;
    for (const char *p = original; *p; p += strlen(p) + 1) {
        if (!_strnicmp(p, "RUSTUP_AUTO_INSTALL=", 20)) continue;
        if (sdk && !_strnicmp(p, "MALVADO_SDK_PREFIX=", 19)) continue;
        entries[used++] = p;
    }
    entries[used++] = "RUSTUP_AUTO_INSTALL=0";
    if (sdk) {
        sdk_entry = malloc(strlen(sdk) + sizeof("MALVADO_SDK_PREFIX="));
        if (!sdk_entry) goto done;
        sprintf(sdk_entry, "MALVADO_SDK_PREFIX=%s", sdk);
        entries[used++] = sdk_entry;
    }
    qsort(entries, used, sizeof(*entries), compare_environment);
    size_t length = 1;
    for (size_t i = 0; i < used; ++i) length += strlen(entries[i]) + 1;
    if (length > 32767) goto done;
    block = malloc(length);
    if (!block) goto done;
    char *cursor = block;
    for (size_t i = 0; i < used; ++i) {
        size_t size = strlen(entries[i]) + 1;
        memcpy(cursor, entries[i], size);
        cursor += size;
    }
    *cursor = '\0';
done:
    free(sdk_entry);
    free(entries);
    FreeEnvironmentStringsA(original);
    return block;
}

int
malvado_windows_process(char *const args[], const char *sdk, char *output, size_t size)
{
    if (!args || !args[0] || (output && !size)) return 1;
    if (output) output[0] = '\0';
    char *line = command_line(args);
    char *environment = child_environment(sdk);
    if (!line || !environment) { free(line); free(environment); return 1; }
    HANDLE reader = NULL, writer = NULL;
    SECURITY_ATTRIBUTES security = {sizeof(security), NULL, TRUE};
    if (output && (!CreatePipe(&reader, &writer, &security, 0) ||
                   !SetHandleInformation(reader, HANDLE_FLAG_INHERIT, 0))) goto fail;
    STARTUPINFOA startup = {0};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startup.hStdOutput = output ? writer : GetStdHandle(STD_OUTPUT_HANDLE);
    PROCESS_INFORMATION process = {0};
    if (!CreateProcessA(NULL, line, NULL, NULL, TRUE, 0, environment, NULL,
                        &startup, &process)) {
        fprintf(stderr, "Cannot execute %s (Windows error %lu)\n", args[0], GetLastError());
        goto fail;
    }
    free(line);
    free(environment);
    CloseHandle(process.hThread);
    int ok = 1;
    if (output) {
        CloseHandle(writer);
        size_t used = 0;
        char buffer[512];
        DWORD count;
        while (ReadFile(reader, buffer, sizeof(buffer), &count, NULL) && count) {
            if ((size_t)count >= size - used) ok = 0;
            else if (ok) { memcpy(output + used, buffer, count); used += count; }
        }
        if (GetLastError() != ERROR_BROKEN_PIPE) ok = 0;
        output[used] = '\0';
        CloseHandle(reader);
    }
    DWORD code = 1;
    if (WaitForSingleObject(process.hProcess, INFINITE) != WAIT_OBJECT_0 ||
        !GetExitCodeProcess(process.hProcess, &code)) ok = 0;
    CloseHandle(process.hProcess);
    return ok && code <= INT32_MAX ? (int)code : 1;
fail:
    if (reader) CloseHandle(reader);
    if (writer) CloseHandle(writer);
    free(line);
    free(environment);
    return 1;
}
