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

#include <stdio.h>
#include "malvado/malvado.h"

typedef struct {
    const char *name;
    int step;
} Worker;

static MvStepResult
work(MvContext *game, MvProcessId id, void *user)
{
    (void)game;
    (void)id;
    Worker *worker = user;
    printf("%s: %d\n", worker->name, ++worker->step);
    /* Yield after the first action so the other worker can take its turn. */
    return worker->step == 1 ? MV_STEP_YIELD : MV_STEP_DONE;
}

int
main(void)
{
    MvContext *game = mv_context_create(60, 5);
    if (!game)
        return 1;

    Worker first = {"first", 0};
    Worker second = {"second", 0};
    MvProcessId id;
    int success = mv_process_spawn(game, 0, 0, work, NULL, &first, &id) == MV_OK &&
                  mv_process_spawn(game, 0, 0, work, NULL, &second, &id) == MV_OK;
    /* Keep both workers in scope until the context releases their callbacks. */
    while (success && mv_process_count(game))
        success = mv_process_tick(game) == MV_OK;
    mv_context_destroy(game);
    return success ? 0 : 1;
}
