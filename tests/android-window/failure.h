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

#ifndef MALVADO_TEST_ANDROID_FAILURE_H
#define MALVADO_TEST_ANDROID_FAILURE_H

static int failure_process_destroyed, failure_image_destroyed;

static void
failure_process_destroy(void *data)
{
    assert(data == &failure_process_destroyed);
    failure_process_destroyed++;
}

static void
failure_image_destroy(void *data)
{
    assert(data);
    mv_image_destroy(data);
    failure_image_destroyed++;
}

static MvStepResult
failure_step(MvContext *ctx, MvProcessId id, void *data)
{
    (void)data;
    MvProcessRef owner;
    MvResource *resource;
    static const uint8_t pixel[] = {255, 0, 0, 255};
    MvImage *image = mv_image_from_rgba(1, 1, pixel, sizeof(pixel));
    assert(image);
    assert(mv_process_ref_create(ctx, id, &owner) == MV_OK);
    assert(mv_resource_create(ctx, &owner, image, failure_image_destroy, &resource) == MV_OK);
    assert(ACCEPTANCE_FAILURE == 1);
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s C process error observed", ACCEPTANCE_TOKEN);
    return MV_STEP_ERROR;
}

static void
failure_run(void)
{
    failure_process_destroyed = failure_image_destroyed = 0;
    MvContext *ctx = mv_context_create(60, 4);
    MvProcessId id;
    assert(ctx);
    assert(mv_process_spawn(ctx, 0, 0, failure_step, failure_process_destroy,
                            &failure_process_destroyed, &id) == MV_OK);
    assert(mv_process_tick(ctx) == MV_PROCESS_ERROR);
    assert(mv_process_count(ctx) == 0);
    assert(failure_process_destroyed == 1 && failure_image_destroyed == 1);
    mv_context_destroy(ctx);
    assert(failure_process_destroyed == 1 && failure_image_destroyed == 1);
    mv_window_close();
    assert(!IsWindowReady() && !IsAudioDeviceReady());
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s C failure resources closed", ACCEPTANCE_TOKEN);
    __android_log_print(ANDROID_LOG_INFO, "MalvadoAcceptance", "%s C failure cleanup passed", ACCEPTANCE_TOKEN);
}
#endif
