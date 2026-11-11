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
#include "malvado/platform.h"
#include <assert.h>
#include <stdlib.h>

/* Fake native objects isolate resource ownership from device behavior. */
struct MvImage { int destroyed; };
struct MvFont { int destroyed; };
struct MvSound { int destroyed; };
struct MvMusic { int destroyed; };
void mv_image_destroy(MvImage *data) { assert(!data->destroyed++); }
void mv_font_destroy(MvFont *data) { assert(!data->destroyed++); }
void mv_sound_destroy(MvSound *data) { assert(!data->destroyed++); }
void mv_music_destroy(MvMusic *data) { assert(!data->destroyed++); }
static MvStepResult idle(MvContext *ctx, MvProcessId id, void *user)
{
    (void)ctx;
    (void)id;
    (void)user;
    return MV_STEP_YIELD;
}
typedef struct { MvContext *ctx; int called; } ClosingCheck;
static void check_closing(void *user)
{
    ClosingCheck *check = user;
    struct MvSound sound = {0};
    MvResource *out = NULL;
    assert(mv_resource_adopt_sound(check->ctx, NULL, &sound, &out) == MV_BUSY);
    assert(!out && !sound.destroyed);
    mv_sound_destroy(&sound);
    assert(sound.destroyed == 1);
    ++check->called;
}

int main(void)
{
    MvContext *ctx = mv_context_create(60, 4), *foreign = mv_context_create(60, 4);
    assert(ctx && foreign);
    MvProcessId id;
    MvProcessRef owner;
    assert(mv_process_spawn(ctx, 0, 0, idle, NULL, NULL, &id) == MV_OK);
    assert(mv_process_ref_create(ctx, id, &owner) == MV_OK);
    struct MvImage image = {0}, cached_reference = {0};
    struct MvFont font = {0};
    struct MvSound sound = {0}, clone = {0};
    struct MvMusic music = {0};
    MvResource *images, *fonts, *sounds, *clones, *streams;
    MvResource *out = NULL;
    assert(mv_resource_adopt_image(ctx, NULL, NULL, &out) == MV_INVALID_ARGUMENT && !out);
    assert(mv_resource_adopt_font(ctx, NULL, NULL, &out) == MV_INVALID_ARGUMENT && !out);
    assert(mv_resource_adopt_sound(ctx, NULL, NULL, &out) == MV_INVALID_ARGUMENT && !out);
    assert(mv_resource_adopt_music(ctx, NULL, NULL, &out) == MV_INVALID_ARGUMENT && !out);
    assert(mv_resource_adopt_image(NULL, NULL, &image, &out) == MV_INVALID_ARGUMENT && !out);
    assert(mv_resource_adopt_font(foreign, &owner, &font, &out) == MV_INVALID_ARGUMENT && !out);
    assert(mv_resource_adopt_sound(ctx, NULL, &sound, NULL) == MV_INVALID_ARGUMENT);
    assert(!image.destroyed && !font.destroyed && !sound.destroyed);
    assert(mv_resource_adopt_image(ctx, &owner, &image, &images) == MV_OK);
    out = images;
    assert(mv_resource_adopt_font(foreign, &owner, &font, &out) == MV_INVALID_ARGUMENT && out == images);
    assert(mv_resource_adopt_font(ctx, &owner, &font, &fonts) == MV_OK);
    assert(mv_resource_adopt_sound(ctx, &owner, &sound, &sounds) == MV_OK);
    assert(mv_resource_adopt_sound(ctx, &owner, &clone, &clones) == MV_OK);
    assert(mv_resource_adopt_music(ctx, NULL, &music, &streams) == MV_OK);
    assert(mv_resource_retain(images) == MV_OK);
    assert(mv_resource_retain(fonts) == MV_OK);
    assert(mv_resource_retain(streams) == MV_OK);
    assert(mv_resource_transfer(clones, NULL) == MV_OK);
    assert(mv_resource_data(images) == &image && mv_resource_data(fonts) == &font);
    int killed;
    assert(mv_process_kill(ctx, owner, &killed) == MV_OK && killed);
    assert(mv_process_tick(ctx) == MV_OK);
    assert(sound.destroyed == 1 && !image.destroyed && !font.destroyed && !clone.destroyed);
    assert(mv_resource_adopt_image(ctx, &owner, &cached_reference, &out) == MV_NOT_FOUND && out == images);
    assert(!cached_reference.destroyed);
    assert(mv_resource_release(images) == MV_OK && image.destroyed == 1);
    ClosingCheck check = {ctx, 0};
    assert(mv_process_spawn(ctx, 0, 0, idle, check_closing, &check, &id) == MV_OK);
    mv_context_destroy(foreign);
    mv_context_destroy(ctx);
    assert(check.called == 1);
    assert(clone.destroyed == 1 && !font.destroyed && !music.destroyed);
    assert(mv_resource_data(fonts) == &font && mv_resource_data(streams) == &music);
    assert(mv_resource_release(fonts) == MV_OK && font.destroyed == 1);
    assert(mv_resource_release(streams) == MV_OK && music.destroyed == 1);
    /* A rejected adoption leaves destruction with the caller. */
    mv_image_destroy(&cached_reference);
    assert(cached_reference.destroyed == 1);
    return 0;
}
