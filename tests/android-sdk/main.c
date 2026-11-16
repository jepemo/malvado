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

#include "malvado/malvado.h"
#include "malvado/asset.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int steps;
    int destroyed;
} Counter;

static MvStepResult
step(MvContext *context, MvProcessId id, void *user)
{
    (void)context;
    (void)id;
    Counter *counter = user;
    counter->steps++;
    return counter->steps == 2 ? MV_STEP_DONE : MV_STEP_YIELD;
}

static void
destroy(void *user)
{
    Counter *counter = user;
    counter->destroyed++;
}

int
main(void)
{
    assert(mv_abi_version() == 2);
    assert(!strcmp(mv_version_string(), EXPECTED_VERSION));
    MvContext *context = mv_context_create(60, 5);
    assert(context);
    Counter counter = {0};
    MvProcessId id;
    MvProcessRef reference;
    MvProcessHandle *handle;
    MvProcessHandleInfo info;
    assert(mv_process_spawn(context, 0, 0, step, destroy, &counter, &id) == MV_OK);
    assert(mv_process_ref_create(context, id, &reference) == MV_OK);
    assert(mv_process_handle_create(context, reference, &handle) == MV_OK);
    assert(mv_process_tick(context) == MV_OK);
    assert(counter.steps == 1 && counter.destroyed == 0);
    assert(mv_process_tick(context) == MV_OK);
    assert(counter.steps == 2 && counter.destroyed == 1);
    assert(mv_process_count(context) == 0);
    assert(mv_process_handle_snapshot(handle, &info) == MV_OK);
    assert(!info.alive && info.reason == MV_END_RETURNED);
    mv_context_destroy(context);
    assert(mv_process_handle_snapshot(handle, &info) == MV_OK && !info.alive);
    mv_process_handle_destroy(handle);

    /* A source snapshot owns its bytes after the file disappears. */
    const unsigned char bytes[] = {0, 1, 13, 10, 127, 255};
    FILE *file = fopen("binary asset with spaces.bin", "wb");
    assert(file && fwrite(bytes, 1, sizeof(bytes), file) == sizeof(bytes));
    assert(fclose(file) == 0);
    MvAssetSource *source = mv_asset_source_open("binary asset with spaces.bin", NULL);
    assert(source);
    assert(remove("binary asset with spaces.bin") == 0);
    size_t length;
    const void *snapshot = mv_asset_source_data(source, &length);
    assert(length == sizeof(bytes) && !memcmp(snapshot, bytes, length));
    mv_asset_source_destroy(source);

    /* Shutdown must also destroy an unfinished process exactly once. */
    context = mv_context_create(60, 5);
    assert(context);
    Counter unfinished = {0};
    assert(mv_process_spawn(context, 0, 0, step, destroy, &unfinished, &id) == MV_OK);
    assert(mv_process_tick(context) == MV_OK && unfinished.steps == 1);
    mv_context_destroy(context);
    assert(unfinished.destroyed == 1);
    puts("Android installed SDK: process, handle, asset and shutdown passed");
    return 0;
}
