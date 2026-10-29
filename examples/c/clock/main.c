/* Malvado 2 - Jeremías Pérez and contributors. SPDX-License-Identifier: Apache-2.0 */
#include <stdio.h>
#include "malvado/malvado.h"

int
main(void)
{
    MvContext *game = mv_context_create(60, 5);
    if (!game)
        return 1;

    unsigned steps = 0;
    if (mv_context_advance(game, 1.0 / 30.0, &steps) != MV_OK) {
        mv_context_destroy(game);
        return 1;
    }
    printf("Hola, Malvado C: %u ticks\n", steps);
    mv_context_destroy(game);
    return steps == 2 ? 0 : 1;
}
