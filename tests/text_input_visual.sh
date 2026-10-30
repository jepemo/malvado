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
xdotool type --clearmodifiers --delay 50 'aZ9'
wait "$pid"
trap - EXIT
