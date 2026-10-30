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
xdotool keydown Left keydown Right keydown Up keydown Down keydown a keydown 0 keydown space
sleep 0.4
xdotool keyup Left keyup Right keyup Up keyup Down keyup a keyup 0 keyup space
sleep 0.4
xdotool keydown Escape
sleep 0.3
xdotool keyup Escape
wait "$pid"
trap - EXIT
