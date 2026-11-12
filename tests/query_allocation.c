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


#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include "malvado/malvado.h"
#include "malvado/collision.h"
#include "malvado/graphic.h"

static int fail_allocation;
static int allocations_before_failure = -1;

void *test_query_malloc(size_t size)
{
    if (fail_allocation || allocations_before_failure == 0) return NULL;
    if (allocations_before_failure > 0) --allocations_before_failure;
    return malloc(size);
}

static void
count_release(void *data)
{
    int *count = data;
    ++*count;
}

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
    assert(ctx);
    MvProcessId root, child, independent;
    assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &root) == MV_OK);
    assert(mv_process_spawn(ctx, root, 0, idle, NULL, NULL, &child) == MV_OK);
    assert(mv_process_spawn_related(ctx, root, 0, 0, idle, NULL, NULL, &independent) == MV_OK);
    MvProcessRef root_ref, child_ref;
    assert(mv_process_ref_create(ctx, root, &root_ref) == MV_OK);
    assert(mv_process_ref_create(ctx, child, &child_ref) == MV_OK);
    MvCollisionShape shape = {.kind = MV_COLLISION_CIRCLE, .radius = 1};
    MvCollisionTransform transform = {0, 0, 0, 1};
    char payload[] = "payload";
    MvMessage bytes = {.type = MV_MESSAGE_BYTES, .value.bytes = {payload, sizeof(payload)}};
    int releases = 0;
    MvMessage opaque = {.type = MV_MESSAGE_OPAQUE, .value.opaque = {&releases, count_release}};
    for (int fail_at = 0; fail_at < 2; ++fail_at) {
        allocations_before_failure = fail_at;
        assert(mv_process_send(ctx, root, &bytes) == MV_OUT_OF_MEMORY);
        allocations_before_failure = -1;
        size_t count = 99;
        assert(mv_process_message_count(ctx, root, &count) == MV_OK && count == 0);
        assert(payload[0] == 'p');
    }
    allocations_before_failure = 0;
    assert(mv_process_send(ctx, root, &opaque) == MV_OUT_OF_MEMORY);
    allocations_before_failure = -1;
    assert(releases == 0);
    assert(mv_process_send(ctx, root, &opaque) == MV_OK);
    MvMessage received;
    assert(mv_process_receive(ctx, root, &received) == MV_OK);
    mv_message_release(&received);
    assert(releases == 1);
    assert(mv_process_send(ctx, root, &bytes) == MV_OK);
    assert(mv_process_receive(ctx, root, &received) == MV_OK);
    assert(received.type == MV_MESSAGE_BYTES && received.value.bytes.size == sizeof(payload));
    mv_message_release(&received);
    fail_allocation = 1;
    assert(mv_process_collider_set(ctx, root_ref, shape, transform) == MV_OK);
    assert(mv_process_collider_set(ctx, child_ref, shape, transform) == MV_OK);
    MvProcessRef matches[1];
    size_t match_count;
    assert(mv_process_collider_find(ctx, root_ref, 0, matches, 1, &match_count) == MV_OK);
    assert(match_count == 1 && matches[0].id == child);
    MvProcessRef first;
    assert(mv_process_collider_first(ctx, root_ref, 0, &first) == MV_OK && first.id == child);
    MvProcessInfo info;
    assert(mv_process_info(ctx, first, &info) == MV_OK && info.id == child);
    assert(mv_process_collider_clear(ctx, child_ref) == MV_OK);
    assert(mv_process_signal(ctx, root, MV_SIGNAL_FREEZE, 1) == MV_OUT_OF_MEMORY);
    assert(mv_process_signal_creation_tree(ctx, root, MV_SIGNAL_KILL) == MV_OUT_OF_MEMORY);
    fail_allocation = 0;
    MvProcessInfo items[3];
    size_t total;
    assert(mv_process_snapshot(ctx, items, 3, &total) == MV_OK && total == 3);
    for (size_t i = 0; i < total; i++)
        assert(items[i].status == MV_PROCESS_RUNNING);
    assert(mv_process_signal_creation_tree(ctx, root, MV_SIGNAL_FREEZE) == MV_OK);
    assert(mv_process_snapshot(ctx, items, 3, &total) == MV_OK && total == 3);
    for (size_t i = 0; i < total; i++)
        assert(items[i].status == MV_PROCESS_FROZEN);
    for (int failure = 0; failure < 2; ++failure) {
        allocations_before_failure = failure;
        assert(mv_process_draw_text(ctx,root_ref,"copy",0,0,20,(MvColor){0}) == MV_OUT_OF_MEMORY);
        allocations_before_failure = -1;
        size_t commands = 99;
        assert(mv_process_draw_count(ctx,root_ref,&commands) == MV_OK && commands == 0);
    }
    mv_context_destroy(ctx);
    return 0;
}
