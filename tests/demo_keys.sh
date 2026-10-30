#!/bin/sh
# Malvado 2; created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

set -eu
MALVADO_REAL_INPUT=1 "$1" "$2" &
pid=$!
trap 'kill "$pid" 2>/dev/null || true' EXIT
window=$(timeout 10 xdotool search --sync --name '^Malvado Demo$' | head -n 1)
xdotool windowfocus --sync "$window"
xdotool mousemove --window "$window" 320 235 mousedown 1
sleep 0.2
xdotool mouseup 1
sleep 0.3
xdotool keydown Escape
sleep 0.2
xdotool keyup Escape
sleep 0.3
xdotool keydown Down
sleep 0.2
xdotool keyup Down
sleep 0.2
xdotool keydown Return
sleep 0.2
xdotool keyup Return
sleep 0.3
xdotool keydown Escape
sleep 0.2
xdotool keyup Escape
sleep 0.3
xdotool keydown Escape
sleep 0.2
xdotool keyup Escape
wait "$pid"
trap - EXIT
