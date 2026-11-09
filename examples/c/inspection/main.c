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
#include <stdio.h>
#include "malvado/malvado.h"

static MvStepResult
idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}

int
main(void)
{
    MvContext *ctx = mv_context_create(60, 5);
    if (!ctx)
        return 1;
    MvProcessId root, owned, independent;
    MvProcessRef owned_ref;
    MvProcessInfo processes[3], info;
    MvLineageInfo lineage[3];
    MvInspectionHeader header;
    size_t count = 0, process_total = 0, lineage_total = 0;
    int ok = mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &root) == MV_OK &&
        mv_process_spawn(ctx, root, 0, idle, NULL, NULL, &owned) == MV_OK &&
        mv_process_spawn_related(ctx, root, 0, 0, idle, NULL, NULL, &independent) == MV_OK &&
        mv_process_set_type(ctx, owned, 7) == MV_OK &&
        mv_process_set_type(ctx, independent, 7) == MV_OK &&
        mv_process_ref_create(ctx, owned, &owned_ref) == MV_OK &&
        mv_process_info(ctx, owned_ref, &info) == MV_OK && info.parent == root &&
        mv_process_snapshot(ctx, processes, 3, &count) == MV_OK && count == 3 &&
        mv_process_find(ctx, 7, processes, 3, &count) == MV_OK && count == 2 &&
        mv_process_children(ctx, root, 0, processes, 3, &count) == MV_OK && count == 1 &&
        processes[0].id == owned &&
        mv_process_descendants(ctx, root, 0, processes, 3, &count) == MV_OK && count == 1 &&
        mv_process_creation_children(ctx, root, 0, processes, 3, &count) == MV_OK && count == 2 &&
        processes[0].id == owned && processes[1].id == independent &&
        mv_process_creation_descendants(ctx, root, 0, processes, 3, &count) == MV_OK && count == 2 &&
        mv_process_lineage_snapshot(ctx, lineage, 3, &count) == MV_OK && count == 3 &&
        mv_process_inspection_snapshot(ctx, processes, 3, &process_total,
                                       lineage, 3, &lineage_total) == MV_OK &&
        process_total == 3 && lineage_total == 3 &&
        mv_process_inspection_capture(ctx, &header, NULL, 0, &process_total,
                                      NULL, 0, &lineage_total) == MV_BUFFER_TOO_SMALL &&
        process_total == 3 && lineage_total == 3 &&
        mv_process_inspection_capture(ctx, &header, processes, 3, &process_total,
                                      lineage, 3, &lineage_total) == MV_OK && header.tick == 0;
    if (ok)
        puts("Three processes: one owned child, two creation children");
    mv_context_destroy(ctx);
    return ok ? 0 : 1;
}
