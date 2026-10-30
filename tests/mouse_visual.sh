#!/bin/sh
# Malvado 2; created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

set -eu
title=$1
shift
"$@" &
pid=$!
trap 'kill "$pid" 2>/dev/null || true' EXIT
window=$(timeout 10 xdotool search --sync --name "^$title$" | head -n 1)
xdotool windowfocus --sync "$window"
xdotool mousemove --window "$window" 50 45
xdotool mousedown 1 mousedown 2 mousedown 3
sleep 0.4
xdotool mouseup 1 mouseup 2 mouseup 3
wait "$pid"
trap - EXIT
