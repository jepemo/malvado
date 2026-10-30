#!/bin/sh
# Malvado 2; created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

set -eu
unset MALVADO_SMOKE
MALVADO_INPUT_TEST=1 "$1" "$2" &
pid=$!
trap 'kill "$pid" 2>/dev/null || true' EXIT
window=$(timeout 10 xdotool search --sync --name '^Space Asteroids$' | head -n 1)
xdotool windowfocus --sync "$window"
xdotool keydown Left keydown Up keydown space
sleep 0.4
xdotool keyup Left keyup Up keyup space
xdotool keydown Escape
sleep 0.3
xdotool keyup Escape
wait "$pid"
trap - EXIT
