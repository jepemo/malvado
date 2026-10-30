#!/bin/sh
# Malvado 2; created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

set -eu
output=$(mktemp)
"$1" "$2" > "$output" 2>&1 &
pid=$!
trap 'kill "$pid" 2>/dev/null || true; rm -f "$output"' EXIT
window=$(timeout 10 xdotool search --sync --name '^Benchmark$' | head -n 1)
xdotool windowfocus --sync "$window"
xdotool keydown space
sleep 0.4
xdotool keyup space
xdotool keydown Escape
sleep 0.3
xdotool keyup Escape
wait "$pid"
grep -E 'benchmark stopped: [1-9][0-9]* processes' "$output"
trap - EXIT
rm -f "$output"
