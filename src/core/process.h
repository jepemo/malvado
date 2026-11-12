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
#ifndef MALVADO_PROCESS_H
#define MALVADO_PROCESS_H
#include "malvado/malvado.h"
typedef struct MvProcesses MvProcesses;
/* A dead process is available until cleanup or through the most recent tick. */
int mv_process_end_reason(MvContext *ctx, MvProcessId id, MvEndReason *reason);
/* Read the next ID without reserving it. The caller must not yield before spawn. */
MvResult mv_process_next_id(MvContext *ctx, MvProcessId *out);
MvProcesses *mv_processes_create(MvContext *ctx);
void mv_processes_destroy(MvProcesses *processes);
int mv_processes_can_destroy(const MvProcesses *processes);
MvProcesses *mv_context_processes(MvContext *ctx);
/* Private platform graph ownership. Success takes data; callbacks must only
 * release platform objects and must not call the process/context API. */
#include "malvado/collision.h"
MvResult malvado_process_visual_set(MvContext *ctx, MvProcessRef ref,
    MvCollisionShape shape, void *data, MvResourceDestroy destroy);
MvResult malvado_process_visual_get(MvContext *ctx, MvProcessRef ref, void **data);
#include "malvado/animation.h"
typedef int (*MvVisualSample)(void *data, size_t index, MvCollisionShape *shape);
/* Sample must only select immutable platform values, with no process calls. */
MvResult malvado_process_visual_animation_set(MvContext *ctx, MvProcessRef ref,
    MvAnimation *animation, unsigned rate, void *data, MvResourceDestroy destroy,
    MvVisualSample sample);
/* Binding-only identity check. The caller retains the compared definition. */
int malvado_process_visual_animation_uses(MvContext *ctx, MvProcessRef ref,
                                        const MvAnimation *definition);
/* Private immutable command records. Visitors may only draw borrowed values;
 * they must not allocate Lua objects or call process/context operations. Owned
 * records transfer data only on success; destroy only releases platform data. */
#include "malvado/graphic.h"
typedef enum { MV_STORED_BOX, MV_STORED_CIRCLE, MV_STORED_TEXT, MV_STORED_CUSTOM } MvStoredDrawKind;
typedef struct {
    MvStoredDrawKind kind;
    int x, y, width, height, radius, size, filled;
    MvColor color;
    const char *text;
    void *data;
    MvResourceDestroy destroy;
    int (*draw)(const void *data);
} MvStoredDraw;
typedef void (*MvDrawVisitor)(const MvStoredDraw *command, void *user);
MvResult malvado_process_draw_owned(MvContext *ctx, MvProcessRef process, MvStoredDraw command);
MvResult malvado_process_draw_visit(MvContext *ctx, MvProcessRef process,
    MvDrawVisitor visit, void *user);
/* Binding/platform-only retained text renderer. Callbacks must only release or
 * draw borrowed platform data; they must not call context/process/text APIs.
 * Successful bind takes data; replacement/removal/cleanup releases once.
 * All NULL callbacks/data clear the binding. Other partial tuples are invalid. */
#include "malvado/text.h"
typedef int (*MvTextDraw)(const void *data, const MvText *text);
MvResult malvado_text_entry_bind(MvContext *ctx, uint64_t id, void *data,
    MvResourceDestroy destroy, MvTextDraw draw);
/* Borrow data only when the callback identity matches; otherwise write NULL.
 * Borrow ends at replacement/removal/cleanup. Output required. */
MvResult malvado_text_entry_binding(MvContext *ctx, uint64_t id, MvTextDraw draw, void **out);
int malvado_text_entries_draw(MvContext *ctx);
#endif
