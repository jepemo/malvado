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
#include "malvado/animation.h"

typedef struct {
    const MvAnimation *definition;
    MvAnimationState playback;
    unsigned tick;
} Player;

static MvStepResult
animate(MvContext *context, MvProcessId id, void *user)
{
    (void)context;
    (void)id;
    Player *player = user;
    MvAnimationSample sample;
    if (!mv_animation_advance(player->definition, &player->playback, 60) ||
        !mv_animation_sample(player->definition, player->playback.elapsed_us, &sample))
        return MV_STEP_ERROR;
    ++player->tick;
    if (player->tick == 6 && sample.graph_id != 42) return MV_STEP_ERROR;
    if (player->tick == 12 && sample.graph_id != 7) return MV_STEP_ERROR;
    if (player->tick == 1 || player->tick == 6 || player->tick == 12)
        printf("tick %u: graph %d\n", player->tick, sample.graph_id);
    return player->tick < 12 ? MV_STEP_YIELD : MV_STEP_DONE;
}

int main(void)
{
    MvAnimationFrame frames[] = {{7, 100000}, {42, 100000}};
    MvAnimation *definition = mv_animation_create(frames, 2, 1);
    MvContext *game = mv_context_create(60, 5);
    if (!definition || !game) {
        mv_animation_destroy(definition);
        mv_context_destroy(game);
        return 1;
    }
    Player player = {.definition = definition};
    MvProcessId id;
    int ok = mv_process_spawn(game, 0, 0, animate, NULL, &player, &id) == MV_OK;
    while (ok && mv_process_count(game))
        ok = mv_process_tick(game) == MV_OK;
    ok = ok && player.tick == 12;
    /* The context stops all users before the caller releases the definition. */
    mv_context_destroy(game);
    mv_animation_destroy(definition);
    return ok ? 0 : 1;
}
