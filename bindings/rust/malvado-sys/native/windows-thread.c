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

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>

/* The initial application thread is the oldest live process thread. Rust
 * terminates the process when main returns. Reject incomplete inventories or
 * ambiguous creation times instead of assigning graphics ownership to a worker.
 * No thread ID is cached and every native handle is closed. */
int
malvado_rust_main_thread(void)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    THREADENTRY32 entry = {0};
    entry.dwSize = sizeof(entry);
    FILETIME oldest = {MAXDWORD, MAXDWORD};
    DWORD owner = 0;
    int unique = 0;
    int ok = Thread32First(snapshot, &entry) != 0;
    if (ok) {
        do {
            if (entry.th32OwnerProcessID != GetCurrentProcessId()) continue;
            HANDLE thread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ThreadID);
            if (!thread) { ok = 0; break; }
            FILETIME created, exited, kernel, user;
            int queried = GetThreadTimes(thread, &created, &exited, &kernel, &user) != 0;
            CloseHandle(thread);
            if (!queried) { ok = 0; break; }
            LONG order = CompareFileTime(&created, &oldest);
            if (order < 0) {
                oldest = created;
                owner = entry.th32ThreadID;
                unique = 1;
            } else if (order == 0) {
                unique = 0;
            }
        } while (Thread32Next(snapshot, &entry));
        if (ok && GetLastError() != ERROR_NO_MORE_FILES) ok = 0;
    }
    CloseHandle(snapshot);
    return ok && unique && owner == GetCurrentThreadId();
}
#endif

